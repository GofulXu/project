//////////////////////////////////////////////////////////////////
//
//  File name: GPLE/wechat.c
//
//  Author: wenwang xu (徐文旺)  
//
//  Date: 2017-5
//  
//  Description: 基于ARM平台的音视频传输案例
//
//  Bug Report: ghu56430@qq.com
//
//////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>
#include <unistd.h>
#include <pthread.h>

/*************************************************
				非系统头文件与宏定义
 *************************************************/
#include "lcd.h"
#include "audio.h"
#include "V4L2.h"
#include "network.h"
#include "sound.h"
#include "ts.h"
#include "wechatdef.h"
#include "zigbee.h"
#include "uart.h"
#include "hand.h"
#include "get_time.h"








/*************************************************
				   变量和函数声明
 *************************************************/
static void *thread_ts 			(void *parg);
static void *thread_udp_jpg		(void *parg);
static void *thread_pcm_rcv		(void *parg);
static void *thread_udp_sta		(void *parg);
static void *thread_local_jpg	(void *parg);
static void *thread_pcm_send(void * parg);
static void *button_event(void *parg);
static void *thread_sound_control(void *arg);




static char	g_udp_recv_jpg_buf[48*1024]={0};
static char	g_udp_recv_wav_buf[48*1024]={0};


static char g_dest_ip_buf[32]={0};
TS_DATA robot_ts_data = {0,0,0,0,0,0,0,0};	//触摸相关参数
char send_msg[1024] = {0};
int error_msgfd;					//日志文件
char yeelink_cond = 0;				//云端数据上传标志
static int power_cond = 1;			//电源标志
unsigned char tm_cond = 0;			//空闲时间标志

/* 摄像头状态标志位 */
static volatile int g_video_chat_ctrl_flag 	= VIDEO_CHAT_JPG_IN_JPG_OFF;

static volatile int g_audio_chat_sta		= AUDIO_CHAT_NONE;


/* extern void wave_end(int fd,pcm_format *wav); */
extern snd_pcm_uframes_t read_pcm_data(pcm_container *sound, snd_pcm_uframes_t frames);
extern ssize_t write_pcm_to_device(pcm_container *pcm, size_t wcount); 
extern int get_mwav_header_info(int fd, wav_format *wav);

extern int set_mparams(pcm_container *pcm, wav_format *wav);

extern void play_mmwav(pcm_container *pcm, wav_format *wav, int fd);
extern int play_mwav(char *file_wav);


/* 画中画显示的坐标 */
extern volatile int g_jpg_in_jpg_x;
extern volatile int g_jpg_in_jpg_y;

/* 互斥量:用于保护udp数据收发、jpg图片显示 */
static pthread_mutex_t g_mutex_jpg;
static pthread_mutex_t g_mutex_udp_recv;
static pthread_mutex_t g_mutex_pcm;
static pthread_mutex_t g_mutex_ts_data;

/*************************************************
				      函数区
 *************************************************/


void usage(char *prog)
{
	fprintf(stderr, "Usage: %s <peer's IP>\n", prog);
}




int main(int argc, char **argv)
{
	

	int  i 		= 0;
	int	 len	= 0;
	char sta	= 0;
		
	/* 检查参数个数 */
	if(argc != 2)
	{
		usage(argv[0]);
		exit(0);
	}
	
	/*  获取要设置的目标IP地址 */
	strcpy(g_dest_ip_buf,argv[1]);
	
	
	/* 打开lcd设备 */
	if(-1==lcd_open())
	{
		exit(0);
	}
	
	/* 使能网络udp协议,图像通道 */		
	// udp_open(int socknum, short locdl_port)
	if(-1 == udp_open(UDP_JPG,UDP_PORT_JPG))
	{
		return -1;
	}	
	
	/* 使能网络udp协议,语音通道 */	
	if(-1 == udp_open(UDP_PCM,UDP_PORT_PCM))
	{
		return -1;
	}


	/* 使能网络udp协议,状态通道 */		
	if(-1 == udp_open(UDP_STA,UDP_PORT_STA))
	{
		return -1;
	}		

	error_msgfd = open("./robot_msg.msg", O_RDWR|O_CREAT|O_APPEND);
	
	int set = start_zigbee();				//打开配置zigbee串口，读写信息
	if(set)
	{
		printf("start zigbee error\n");
	}
	
	set = start_hand();						//打开配置机械手串口，读写信息
	if(set)
	{
		printf("start zigbee error\n");
	}
	
	/* 初始化摄像头设备 */
	#if 0
	linux_v4l2_device_init("/dev/video3");
	linux_v4l2_start_capturing();
	#endif

	/* 初始化互斥量 */
	pthread_mutex_init(&g_mutex_jpg		,NULL);
	pthread_mutex_init(&g_mutex_udp_recv,NULL);	
	pthread_mutex_init(&g_mutex_pcm,	NULL);	
	pthread_mutex_init(&g_mutex_ts_data,NULL);	

	//play_mwav("./wav/sound.wav");	
	/* 线程创建 */
	pthread_t id  = 0;
	pthread_create(&id, NULL, thread_ts 		, NULL);
	pthread_create(&id, NULL, thread_local_jpg	, NULL);	
	pthread_create(&id, NULL, thread_udp_jpg	, NULL);	
	pthread_create(&id, NULL, thread_pcm_rcv	, NULL);
	//pthread_create(&id, NULL, thread_pcm_send	, NULL);
	pthread_create(&id, NULL, button_event	, NULL);
	
	int ret = MSP_SUCCESS;
	
	 //讯飞语音用户登录，初始化词表
	ret = sound_init(LOGIN_PARAMS);
	if (MSP_SUCCESS != ret)
	{
		MSPLogout(); // Logout...
		return 0;
	}	
	printf("Uploaded successfully\n");
	
	tm_cond = get_min_time();
	printf("tm_min:%d \n",tm_cond);
	int sockfd = network_server(1234, NULL);	//创建套接字
	//play_mwav("./wav/sound.wav");	
	while(1)
	{	
		
		
		network_accept(sockfd, thread_sound_control);	//监听服务器1234端口，用户客户端信息读写
	
	}	

	
	/* udp关闭 */
	udp_close(UDP_JPG);
	udp_close(UDP_PCM);	

	
	/* lcd关闭 */
	lcd_close();
	
	MSPLogout(); // Logout...退出登录
	return 0;	
}


/********************************************
**接收客户端数据，数据处理
*/
void *thread_sound_control(void *arg)
{
	int sockfd = *((int*)arg);
	char test_buf[1024] = {0}; 
	int ret = read(sockfd, test_buf, 1024);//读取信息
	if(ret < 0)
	{
		perror("recv error");
		return NULL;
	}	
	
	printf("buf:%s\n",test_buf);
	if(test_buf[0] == 'H')			//机械手控制信息处理
	{
		printf("hand_fd\n");
		write(hand_fd, test_buf, strlen(test_buf));
	}else if(test_buf[0] == 'Z')			//zigbee控制信息处理
	{
		printf("zigbee_fd\n");
		write(zigbee_fd, test_buf, strlen(test_buf));
		
	}else if(strcmp(test_buf, "MSG") == 0)		//zigbee读取信息处理
	{		
		write(zigbee_fd, "Zib:MSG", strlen("Zib:MSG"));
		write(hand_fd, "Hand:e", strlen("Hand:e"));
		if(msg.Alarm == '2')	
		{
			memset(send_msg, 0, 1024);
			sprintf(send_msg,"%s#%s", msg.Wor, Ulm_msg);
			printf("send_msg:%s \n", send_msg);
			write(sockfd, send_msg, strlen(send_msg));		//返回报警信息
			
		}else
		{
			//***********test测试***************
			//strcpy(msg.text, "Alarm.0  Temp.23.C  Humi.60%  MQ9.1  MQ135.1  ADC.1111.2222.3333.4444.5555  Power.1000");
			if(strlen(msg.text) < 3){strcpy(msg.text,"wait sometime to config...");}
			if(strlen(Ulm_msg) < 3){strcpy(Ulm_msg,"未获得相关信息");}

			memset(send_msg, 0, 1024);
			sprintf(send_msg,"%s#%s", msg.text, Ulm_msg);
			printf("send_msg:%s \n", send_msg);
			write(sockfd, send_msg, strlen(send_msg));		//返回正常信息
		}
	}else if(strcmp(test_buf, "poweroffnow") == 0)		//zigbee读取信息处理
	{
		power_cond = 0;
	}else if(strstr(test_buf, "Mus:") != NULL)		//zigbee读取信息处理
	{
		switch(test_buf[4])
		{/*
			case 'p':printf("pause");write(hand_fd, "Hand:v", 7);break;
			case '+':printf("sub+");write(hand_fd, "Hand:+", 7);break;
			case '-':printf("sub-");write(hand_fd, "Hand:-", 7);break;
			case 'n':printf("next");write(hand_fd, "Hand:w", 7);break;
			case 'l':printf("last");write(hand_fd, "Hand:x", 7);break;
			case 's':printf("stop");write(hand_fd, "Hand:y", 7);break;
			case 'c':printf("cont");write(hand_fd, "Hand:z", 7);break;
			case 'b':printf("poweron");break;*/
		}
	}else if(strcmp(test_buf,"vidchat:on") == 0)
	{

		//system("killall -9 aplay");
		int ret = voice_Smart_answer(SESSION_BEGIN_PARAMS_ASW, SESSION_BEGIN_PARAMS_WRITE, DOIT_TESTCHAT);
		write(sockfd, g_result, strlen(g_result)+1);
		if (MSP_SUCCESS == ret)
		{
			voice_again(g_result,SESSION_BEGIN_PARAMS_WRITE);
			g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_WAV;
		}else{
			g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_ERROR_WAV1;
		}			
		
	}else if(strcmp(test_buf,"vidrely:on") == 0)
	{
		//system("killall -9 aplay");
		int ret = voice_Smart_answer(SESSION_BEGIN_PARAMS_ASW, SESSION_BEGIN_PARAMS_WRITE, DOIT_VOICE);
		write(sockfd, g_result, strlen(g_result)+1);
		if (MSP_SUCCESS == ret)
		{
			voice_again(g_result,SESSION_BEGIN_PARAMS_WRITE);
			g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_WAV;
		}else{
			g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_ERROR_WAV1;
		}	
	}else if(strcmp(test_buf,"vidbake:on") == 0)
	{
		//system("killall -9 aplay");
		int ret = voice_Smart_answer(SESSION_BEGIN_PARAMS_ASW, SESSION_BEGIN_PARAMS_WRITE, DOIT_TESTBAKE);
		write(sockfd, g_result, strlen(g_result)+1);
		if (MSP_SUCCESS == ret)
		{
			voice_again(g_result,SESSION_BEGIN_PARAMS_WRITE);
			g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_WAV;
		}else{
			g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_ERROR_WAV1;
		}	
	}else if(NULL != strstr(test_buf,"texchat:"))
	{
		//printf("test:%s\n",test_buf);
		if(strlen(test_buf)>10)
		{
			voice_teSmart_answer(test_buf+9, SESSION_BEGIN_PARAMS_ASW, SESSION_BEGIN_PARAMS_WRITE, DOIT_TESTCHAT);
			write(sockfd, g_result, strlen(g_result)+1);
			if(test_buf[8] == 'a')
			{
				//system("killall -9 aplay");
				voice_again(g_result,SESSION_BEGIN_PARAMS_WRITE);
				g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_WAV;
			}
			
		}
		
	}else if(NULL != strstr(test_buf,"texbake:"))
	{
		
		//printf("test:%s\n",test_buf);
		if(strlen(test_buf)>10)
		{
			voice_teSmart_answer(test_buf+9, SESSION_BEGIN_PARAMS_ASW, SESSION_BEGIN_PARAMS_WRITE, DOIT_TESTBAKE);
			write(sockfd, g_result, strlen(g_result)+1);
			if(test_buf[8] == 'a')
			{
				//system("killall -9 aplay");
				voice_again(g_result,SESSION_BEGIN_PARAMS_WRITE);
				g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_WAV;
			}
		}
	}else if(NULL != strstr(test_buf,"texiatt:"))
	{
		
		//printf("test:%s\n",test_buf);
		if(strlen(test_buf)>10)
		{
			voice_teSmart_answer(test_buf+9, SESSION_BEGIN_PARAMS_ASW, SESSION_BEGIN_PARAMS_WRITE, 0);
			write(sockfd, g_result, strlen(g_result)+1);
			if(test_buf[8] == 'a')
			{
				//system("killall -9 aplay");
				voice_again(g_result,SESSION_BEGIN_PARAMS_WRITE);
				g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_WAV;
			}
		}
	}else if(NULL != strstr(test_buf,"send_data:"))	//send_data:temp:22:humi:33:#
	{
		if(yeelink_cond)
		{
			send_yeelink(test_buf);
		}
	}else if(0 == strcmp(test_buf,"yeelink:on"))	//send_data:temp:22:humi:33:#
	{
		yeelink_cond = 1;
	}else if(0 == strcmp(test_buf,"yeelink:of"))	//send_data:temp:22:humi:33:#
	{
		yeelink_cond = 0;
	}

	if(0 == strcmp(test_buf,"wechat:one"))
	{
		g_video_chat_ctrl_flag = VIDEO_CHAT_JPG_IN_JPG_ONE;
	}else if(0 == strcmp(test_buf,"wechat:two"))
	{
		g_video_chat_ctrl_flag = VIDEO_CHAT_JPG_IN_JPG_TWO;
	}else if(0 == strcmp(test_buf,"wechat:off"))
	{
		g_video_chat_ctrl_flag = VIDEO_CHAT_JPG_IN_JPG_OFF;
	}
	
	close(sockfd);
	pthread_exit(NULL);
	
	
}


FrameBuffer frame_buffer;

/*****************************
**读取camera数据并显示发送
*/

void *thread_local_jpg	(void *parg)
{
#if 0
	int len=0;

	printf("thread_local_jpg run ok\r\n");	
	
	while(1)
	{
		if(g_video_chat_ctrl_flag != VIDEO_CHAT_JPG_IN_JPG_OFF)
		{
			/* 获取摄像头数据 */
			linux_v4l2_get_fream(&frame_buffer);
			
			/* 上锁Lcd显示图像 */
			pthread_mutex_lock(&g_mutex_jpg);


			if(g_video_chat_ctrl_flag == VIDEO_CHAT_JPG_IN_JPG_ONE)
				/* jpg流解码,显示摄像头捕获的图像 */
				lcd_draw_jpg(g_jpg_in_jpg_x,g_jpg_in_jpg_y,NULL,frame_buffer.buf,frame_buffer.length,1);	
				
			
			/* 解锁Lcd显示图像 */		
			pthread_mutex_unlock(&g_mutex_jpg);	
			
			/* 发送摄像头数据 */
			
			len=udp_send(UDP_JPG,g_dest_ip_buf,UDP_PORT_JPG,frame_buffer.buf,frame_buffer.length);		
		}
	}
#endif
	
	
}

/*****************************
**读取网络图像数据并显示
*/
void *thread_udp_jpg(void *parg)
{
	int udp_recv_length;
	
	printf("thread_udp_jpg run ok\r\n");
	
	while(1)
	{
		if(g_video_chat_ctrl_flag != VIDEO_CHAT_JPG_IN_JPG_OFF)
		{
			/* 获取udp数据 */
			udp_recv_length=udp_recv(UDP_JPG,g_udp_recv_jpg_buf,sizeof g_udp_recv_jpg_buf);	

			/* 检查是否真正地接收到数据 */
			if(udp_recv_length<0)
				continue;
			
			/* 上锁Lcd显示图像 */
			pthread_mutex_lock(&g_mutex_jpg);
			
			if(g_video_chat_ctrl_flag == VIDEO_CHAT_JPG_IN_JPG_ONE)
				/* 画中画显示jpg流解码,显示摄像头捕获的图像 */
				lcd_draw_jpg_in_jpg(80,0,NULL,g_udp_recv_jpg_buf,udp_recv_length,0);	
			
			if(g_video_chat_ctrl_flag == VIDEO_CHAT_JPG_IN_JPG_TWO)
				/* 正常jpg流解码,显示摄像头捕获的图像 */
				lcd_draw_jpg(80,0,NULL,g_udp_recv_jpg_buf,udp_recv_length,0);	
		
			
			/* 解锁Lcd显示图像 */
			pthread_mutex_unlock(&g_mutex_jpg);
		}
	}
	
}

/**************************************
**发送mic-pcm数据
*/
void *thread_pcm_send(void *parg)
{
	printf("thread_pcm_send run ok\r\n");
	while(1)
	{
		if(g_video_chat_ctrl_flag == VIDEO_CHAT_SEND_WAV)
		{
			pcm_container *sound = calloc(1,sizeof(pcm_container));
			/* 上锁pcm音频 */
			pthread_mutex_lock(&g_mutex_pcm);	
			int ret = snd_pcm_open(&sound->handle, "default",SND_PCM_STREAM_CAPTURE, 0);
			set_wav_params(sound);  
			/* 解锁pcm音频 */		
			pthread_mutex_unlock(&g_mutex_pcm);				 
			while(g_video_chat_ctrl_flag == VIDEO_CHAT_SEND_WAV)
			{		
				/* 获取语音数据 */
				snd_pcm_uframes_t n = sound->frames_per_period;

				uint32_t frames_read = read_pcm_data(sound, n);
				/*解锁录制语音 */
					
				/* 发送录音数据*/
				udp_send(UDP_PCM,g_dest_ip_buf,UDP_PORT_PCM,sound->period_buf, frames_read * sound->bytes_per_frame);
			
			}
			// 5：正常关闭PCM设备
			snd_pcm_drain(sound->handle);  
			snd_pcm_close(sound->handle);  
			  
			// 6：释放相关资源
			free(sound->period_buf);  
			free(sound);  
		}else if(g_video_chat_ctrl_flag == AUDIO_CHAT_SEND_WAV)
		{		
			pthread_mutex_lock(&g_mutex_pcm);				 
			int ret = voice_Smart_answer(SESSION_BEGIN_PARAMS_ASW, SESSION_BEGIN_PARAMS_WRITE, DOIT_TESTCHAT);
			pthread_mutex_unlock(&g_mutex_pcm);				 
			if (MSP_SUCCESS == ret)
			{
				voice_again(g_result,SESSION_BEGIN_PARAMS_WRITE);
				g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_WAV;
			}else{
				g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_ERROR_WAV1;
			}
			g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_OFF;
		}
		}
}


/***********************************
**播放对方发送过来的pcm数据和语音识别的结果
*/
void *thread_pcm_rcv(void *parg)
{
	
	printf("thread_pcm_rcv run ok\r\n");
	while(1)
	{
		if(g_video_chat_ctrl_flag == VIDEO_CHAT_PLAY_WAV)
		{
			printf("VIDEO_CHAT_PLAY_WAV \n");
			
			
			pthread_mutex_lock(&g_mutex_pcm);
			play_mwav("/xww/wav/sound.wav");
			pthread_mutex_unlock(&g_mutex_pcm);
			g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_OFF;
		 	
		}else if(g_video_chat_ctrl_flag == VIDEO_CHAT_PLAY_ERROR_WAV1)
		{
			
			pthread_mutex_lock(&g_mutex_pcm);
			play_mwav("/xww/wav/error1.wav");
			pthread_mutex_unlock(&g_mutex_pcm);
			printf("VIDEO_CHAT_PLAY_ERROR_WAV1 \n");
			g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_OFF;
		}else if(g_video_chat_ctrl_flag == VIDEO_CHAT_PLAY_ERROR_WAV2)
		{
			
			pthread_mutex_lock(&g_mutex_pcm);
			play_mwav("/xww/wav/error2.wav");
			pthread_mutex_unlock(&g_mutex_pcm);
			printf("VIDEO_CHAT_PLAY_ERROR_WAV2 \n");
			g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_OFF;
		}else if(g_video_chat_ctrl_flag == VIDEO_CHAT_PLAY_ERROR_WAV3)
		{
			
			pthread_mutex_lock(&g_mutex_pcm);
			play_mwav("/xww/wav/error3.wav");
			pthread_mutex_unlock(&g_mutex_pcm);
			printf("VIDEO_CHAT_PLAY_ERROR_WAV3 \n");
			g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_OFF;
		}else if(g_video_chat_ctrl_flag == VIDEO_CHAT_RECORD_WAV)
		{
			
			int udp_recv_length;	

			pcm_container *sound = calloc(1,sizeof(pcm_container));			
			// 3：以回放方式打开PCM设备	
			//    设置PCM设备参数
			pthread_mutex_lock(&g_mutex_pcm);
			snd_pcm_open(&sound->handle, "default", SND_PCM_STREAM_PLAYBACK, 0); 
			set_wav_params(sound);
			pthread_mutex_unlock(&g_mutex_pcm);
			while(g_video_chat_ctrl_flag == VIDEO_CHAT_RECORD_WAV)
			{
			
				/* 获取udp音频数据 */
				udp_recv_length=udp_recv(UDP_PCM,sound->period_buf,
							 sound->period_buf);		
				if(udp_recv_length<0)
					continue;

				write_pcm_to_device(sound,udp_recv_length/sound->bytes_per_frame);
				// 5：正常关闭PCM设备
			    snd_pcm_drain(sound->handle);  
			    snd_pcm_close(sound->handle);  
			  
			    // 6：释放相关资源
			    free(sound->period_buf);  
			    free(sound);  	
				g_video_chat_ctrl_flag = VIDEO_CHAT_PLAY_OFF;
			}
		}
	}
		
}



/**************************
**监听触摸屏
*/
void *thread_ts(void *parg)
{
	int rt =-1;

	/* 申请触摸屏设备资源，权限可读可写 */
	rt = ts_open_ex();

	/* 若当前不支持触摸屏设备或被占用，则输出错误信息 */	
	if(rt<0)
	{
		perror("open ts error\n");
		
		return ;
	}
	
	printf("thread_ts run ok\r\n");
	
	while(1)
	{
		while(robot_ts_data.ts_button_cond);
		if(ts_xy_get(&robot_ts_data.ts_x, &robot_ts_data.ts_y, 1) != 0)
		{
			continue;
		}
		tm_cond = get_min_time();
		printf("ts_x:%d   ts_y:%d  \n",robot_ts_data.ts_x,robot_ts_data.ts_y);
		//pthread_mutex_lock(&g_mutex_ts_data);
		
		ts_data_init(&robot_ts_data);
		//pthread_mutex_unlock(&g_mutex_ts_data);
		printf("buybuy  \n");
	}	
	
	return 0;
}


/******************************
**处理界面按键事件
*/
void *button_event(void *parg)
{
	while(1)
	{
		if((get_min_time() - tm_cond) >= 2)							//判断空闲时间是否超出
		{
			robot_ts_data.itf_now_num = ROBOT_NO_COND;		
		}
	
		if(!(robot_ts_data.itf_now_num))							//处于初始界面，播放动画
		{
			lcd_draw_jpg(0,0,"/xww/jpg/itf/itf01.jpg",NULL,0,0);
			//usleep(1000);
			lcd_draw_jpg(0,0,"/xww/jpg/itf/itf02.jpg",NULL,0,0);
			//usleep(1000);
			lcd_draw_jpg(0,0,"/xww/jpg/itf/itf03.jpg",NULL,0,0);
			//usleep(1000);
			lcd_draw_jpg(0,0,"/xww/jpg/itf/itf04.jpg",NULL,0,0);
			//usleep(1000);
			lcd_draw_jpg(0,0,"/xww/jpg/itf/itf05.jpg",NULL,0,0);
			//usleep(1000);
		}
		if(robot_ts_data.ts_button_cond)							//按键事件来临-------处理对应事件
		{
			
			if(!(robot_ts_data.itf_now_num))
			{
				robot_ts_data.itf_now_num = ROBOT_PICONE_COND;
				lcd_draw_jpg(0,0,"/xww/jpg/itf/itf1.jpg",NULL,0,0);
				printf("itf_now_num:%d,itf_last_num:%d,itf_next_num:%d,itf_button_type:%d,itf_button_num:%d,%d,%d\n", robot_ts_data.itf_now_num, robot_ts_data.itf_last_num,robot_ts_data.itf_next_num, robot_ts_data.itf_button_type, robot_ts_data.itf_button_num);
				robot_ts_data.ts_button_cond = 0;
				continue;
			}
			printf("itf_now_num: %d\n", robot_ts_data.itf_now_num);
			if(robot_ts_data.itf_now_num == ROBOT_PICONE_COND)
			{
			printf("sssssssssssss \n");
			switch(robot_ts_data.itf_button_num)
			{
				case ts_itf_button_one: lcd_draw_jpg(0,0,"/xww/jpg/itf/demo.jpg",NULL,0,0);robot_ts_data.itf_now_num = ROBOT_PICTWO_COND; break;
				case ts_itf_button_two: printf("itf_now_num:%d,itf_last_num:%d,itf_next_num:%d,itf_button_type:%d,itf_button_num:%d,%d,%d\n", robot_ts_data.itf_now_num, robot_ts_data.itf_last_num,robot_ts_data.itf_next_num, robot_ts_data.itf_button_type, robot_ts_data.itf_button_num);break;
				case ts_itf_button_the: printf("itf_now_num:%d,itf_last_num:%d,itf_next_num:%d,itf_button_type:%d,itf_button_num:%d,%d,%d\n", robot_ts_data.itf_now_num, robot_ts_data.itf_last_num,robot_ts_data.itf_next_num, robot_ts_data.itf_button_type, robot_ts_data.itf_button_num);break;
				case ts_itf_button_fou: printf("itf_now_num:%d,itf_last_num:%d,itf_next_num:%d,itf_button_type:%d,itf_button_num:%d,%d,%d\n", robot_ts_data.itf_now_num, robot_ts_data.itf_last_num,robot_ts_data.itf_next_num, robot_ts_data.itf_button_type, robot_ts_data.itf_button_num);break;
				case ts_itf_button_fiv: printf("itf_now_num:%d,itf_last_num:%d,itf_next_num:%d,itf_button_type:%d,itf_button_num:%d,%d,%d\n", robot_ts_data.itf_now_num, robot_ts_data.itf_last_num,robot_ts_data.itf_next_num, robot_ts_data.itf_button_type, robot_ts_data.itf_button_num);break;
				case ts_itf_button_six: printf("itf_now_num:%d,itf_last_num:%d,itf_next_num:%d,itf_button_type:%d,itf_button_num:%d,%d,%d\n", robot_ts_data.itf_now_num, robot_ts_data.itf_last_num,robot_ts_data.itf_next_num, robot_ts_data.itf_button_type, robot_ts_data.itf_button_num);break;
				
				default:break;
			}
			}else if(robot_ts_data.itf_now_num == ROBOT_PICTWO_COND)
			{
			switch(robot_ts_data.itf_button_num)
			{
				case ts_itf_button_one: lcd_draw_jpg(0,0,"/xww/jpg/itf/itf1.jpg",NULL,0,0);robot_ts_data.itf_now_num = ROBOT_PICONE_COND; break;	//事件处理
				case ts_itf_button_two: printf("itf_now_num:%d,itf_last_num:%d,itf_next_num:%d,itf_button_type:%d,itf_button_num:%d,%d,%d\n", robot_ts_data.itf_now_num, robot_ts_data.itf_last_num,robot_ts_data.itf_next_num, robot_ts_data.itf_button_type, robot_ts_data.itf_button_num);break;
				case ts_itf_button_the: printf("itf_now_num:%d,itf_last_num:%d,itf_next_num:%d,itf_button_type:%d,itf_button_num:%d,%d,%d\n", robot_ts_data.itf_now_num, robot_ts_data.itf_last_num,robot_ts_data.itf_next_num, robot_ts_data.itf_button_type, robot_ts_data.itf_button_num);break;
				case ts_itf_button_fou: printf("itf_now_num:%d,itf_last_num:%d,itf_next_num:%d,itf_button_type:%d,itf_button_num:%d,%d,%d\n", robot_ts_data.itf_now_num, robot_ts_data.itf_last_num,robot_ts_data.itf_next_num, robot_ts_data.itf_button_type, robot_ts_data.itf_button_num);break;
				case ts_itf_button_fiv: printf("itf_now_num:%d,itf_last_num:%d,itf_next_num:%d,itf_button_type:%d,itf_button_num:%d,%d,%d\n", robot_ts_data.itf_now_num, robot_ts_data.itf_last_num,robot_ts_data.itf_next_num, robot_ts_data.itf_button_type, robot_ts_data.itf_button_num);break;
				case ts_itf_button_six: printf("itf_now_num:%d,itf_last_num:%d,itf_next_num:%d,itf_button_type:%d,itf_button_num:%d,%d,%d\n", robot_ts_data.itf_now_num, robot_ts_data.itf_last_num,robot_ts_data.itf_next_num, robot_ts_data.itf_button_type, robot_ts_data.itf_button_num);break;
				
				default:break;
			}
			}else if(robot_ts_data.itf_now_num == ROBOT_PICTHE_COND)
			{
			switch(robot_ts_data.itf_button_num)
			{
				case ts_itf_button_one: break;	//事件处理
				case ts_itf_button_two: break;
				case ts_itf_button_the: break;
				case ts_itf_button_fou: break;
				case ts_itf_button_fiv: break;
				case ts_itf_button_six: break;
				
				default:break;
			}
			}else if(robot_ts_data.itf_now_num == ROBOT_PICFOU_COND)
			{
			switch(robot_ts_data.itf_button_num)
			{
				case ts_itf_button_one: break;	//事件处理
				case ts_itf_button_two: break;
				case ts_itf_button_the: break;
				case ts_itf_button_fou: break;
				case ts_itf_button_fiv: break;
				case ts_itf_button_six: break;
				
				default:break;
			}
			}else if(robot_ts_data.itf_now_num == ROBOT_PICFIV_COND)
			{
			switch(robot_ts_data.itf_button_num)
			{
				case ts_itf_button_one: break;	//事件处理
				case ts_itf_button_two: break;
				case ts_itf_button_the: break;
				case ts_itf_button_fou: break;
				case ts_itf_button_fiv: break;
				case ts_itf_button_six: break;
				
				default:break;
			}
			}else if(robot_ts_data.itf_now_num == ROBOT_PICSIX_COND)
			{
			switch(robot_ts_data.itf_button_num)
			{
				case ts_itf_button_one: break;	//事件处理
				case ts_itf_button_two: break;
				case ts_itf_button_the: break;
				case ts_itf_button_fou: break;
				case ts_itf_button_fiv: break;
				case ts_itf_button_six: break;
				
				default:break;
			}
			}else if(robot_ts_data.itf_now_num == ROBOT_PICSER_COND)
			{
			switch(robot_ts_data.itf_button_num)
			{
				case ts_itf_button_one: break;	//事件处理
				case ts_itf_button_two: break;
				case ts_itf_button_the: break;
				case ts_itf_button_fou: break;
				case ts_itf_button_fiv: break;
				case ts_itf_button_six: break;
				
				default:break;
			}
			}else if(robot_ts_data.itf_now_num == ROBOT_PICEIG_COND)
			{
			//事件处理
			switch(robot_ts_data.itf_button_num)
			{
				/* 进行视频对讲 */
				case ts_itf_button_one: printf("start video...\n");			\
										send_to_gec210("wechat:one");		\
										g_video_chat_ctrl_flag = VIDEO_CHAT_JPG_IN_JPG_ONE; break;	
				/* 停止视频对讲 */						
				case ts_itf_button_two: printf("close the video...\n");		\
										send_to_gec210("wechat:off");		\
										g_video_chat_ctrl_flag = VIDEO_CHAT_JPG_IN_JPG_OFF; break;	
										
				case ts_itf_button_the: break;
				case ts_itf_button_fou: break;
				case ts_itf_button_fiv: break;
				case ts_itf_button_six: break;

				#if 0
				/* 画中画变化的坐标 */
				default: if(ts_x>80 && ts_x<400)			\
							g_jpg_in_jpg_x = ts_x;			\
															\
							if(ts_y>0 && ts_y<240)			\
							g_jpg_in_jpg_y	= ts_y;break;	\
							
				#endif
			}
			}

		robot_ts_data.itf_button_type = not_press;
		robot_ts_data.itf_button_num = not_press;
		robot_ts_data.ts_button_cond = 0;
		}
	}
}







