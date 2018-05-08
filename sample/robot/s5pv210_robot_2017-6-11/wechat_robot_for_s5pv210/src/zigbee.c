#include "zigbee.h"
#include "uart.h"
#include "network.h"
#include "get_time.h"

int zigbee_fd;
pthread_t zigbee_read_t;
Zigbee_Msg msg;



void *zigbee_read_pthread( void *fd_arg )
{
  int nread,nwrite,i;
  char buff[16];
  fd_set rd;
  unsigned int cond = 0;

  /*利用select函数来实现多个串口的读写*/
	while(1)
	{
	  unsigned char zigbee_message[48];
	  FD_ZERO(&rd);
	  FD_SET(zigbee_fd,&rd);
	  while(FD_ISSET(zigbee_fd,&rd))
	  {
		if(select(zigbee_fd+1,&rd,NULL,NULL,NULL) < 0)
		  perror("select error!\n");
		else
		{
		  while((nread = read(zigbee_fd,buff,sizeof(buff)))>0)
		  {  
			//printf("nread = %d,%s\n",nread,buff);
			buff[nread] = '\0';
			strcat(zigbee_message, buff);
			if(buff[nread-1] == '#')
			{
				cond ++;
				//printf("mes:%s\n",zigbee_message);
				//判断帧头，并验证
				char delim[] = ":";
				char *s = strdup(zigbee_message);
				if(strcmp(strsep(&s, delim),"Zig") == 0)		//正常msg
				{
					memset(&msg,0,sizeof(msg));
					//printf("text:%s\n", msg.text);
					//strcpy(msg.text,s);
					//分割信息处理				//"Alarm.0  Temp.23.C  Humi.60%  MQ9.1  MQ135.1  ADC.1111.2222.3333.4444.5555  Power.1000"
					msg.Alarm = *strsep(&s, delim);
					strcpy(msg.Temp, strsep(&s, delim));
					strcpy(msg.Humi, strsep(&s, delim));
					msg.MQ9 = *strsep(&s, delim);
					msg.MQ135 = *strsep(&s, delim);
					strcpy(msg.ADC, strsep(&s, delim));
					strcpy(msg.Power, strsep(&s, delim));
					get_time(msg.time);			
					sprintf(msg.text, "Alarm:%c  Temp:%s.C  Humi:%s%%  MQ9:%c  MQ135:%c  ADC:%s  Power:%s%%  ------  %s", msg.Alarm, msg.Temp, msg.Humi, msg.MQ9, msg.MQ135, msg.ADC, msg.Power, msg.time);
				}			
				printf("text:%s\n", msg.text);
				//判断是否存在异常
				if(msg.Alarm == '2')		//异常处理
				{
					//printf("wor:%s\n", msg.Wor);
					strcat(msg.Wor, "wor:");
					if(atoi(msg.Temp) > TEMP_MAX){strcat(msg.Wor, " Temp.");strcat(msg.Wor, msg.Temp);}
					if(atoi(msg.Humi) > HUMI_MAX){strcat(msg.Wor, " Humi.");strcat(msg.Wor, msg.Humi);}
					if(msg.MQ9 == MQ9_MAX){strcat(msg.Wor, " MQ9.");strcat(msg.Wor, "0");}
					if(msg.MQ135 == MQ135_MAX){strcat(msg.Wor, " MQ135.");strcat(msg.Wor, "0");}
					if(atoi(msg.Power) < POWER_MIN){strcat(msg.Wor, " Power.");strcat(msg.Wor, "0");}
					get_time(msg.time);	
					strcat(msg.Wor, "   ------   ");
					strcat(msg.Wor, msg.time);
					write(error_msgfd, msg.Wor, strlen(msg.Wor));
					printf("wor:%s\n", msg.Wor);
					char buf[32] = {0};
					sprintf(buf, "send_data:temp:%s:humi:%s:#", msg.Temp, msg.Humi);
					//send_to_VIDEO(buf);
				}
				
				if(cond > 10)
				{
					cond = 0;
					char buf[32] = {0};
					sprintf(buf, "send_data:temp:%s:humi:%s:#", msg.Temp, msg.Humi);
					//send_to_VIDEO(buf);
					printf("send_data:%s\n", buf);
				}
				
				//printf("zig:%s\n",zigbee_message);
				memset(zigbee_message,0,sizeof(zigbee_message));
			}
			
		  }
		}
		
		
	  }
	  
	} 
	
	

	close(zigbee_fd);
	pthread_exit( NULL );

}

/*
 *启动连接上zigbee
 *将zigbee信息解析成字符串存在command_arg，
 *
 */
int start_zigbee( void  )
{
	zigbee_fd = open( ZIGBEE_DEV , O_RDWR|O_NOCTTY ) ;
	msg.Alarm == '1';
	uart_init(zigbee_fd);
	
	if( fcntl(zigbee_fd , F_SETFL ,  FNDELAY) < 0)		//非阻塞读写
	{
		perror("fcntl zigbee_fd fail\n");
		exit(1);
	}

	tcdrain(zigbee_fd);
	tcflush(zigbee_fd,TCIOFLUSH);

	if( (pthread_create(&zigbee_read_t , NULL , zigbee_read_pthread , NULL )) != 0)
	{
		perror("Create the zigbee_read_pthread fail");
		exit(1);
	}
	
	return 0;

}




