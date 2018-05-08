#include "hand.h"
#include "uart.h"
#include "network.h"

int hand_fd;
pthread_t hand_read_t;
unsigned char Ulm_msg[32];


void *hand_read_pthread(void *fd_arg)
{
  int nread,nwrite,i;
  char buff[16];
  fd_set rd;
  
  //利用select函数来实现多个串口的读写
	while(1)
	{
	  unsigned char hand_message[16];
	  FD_ZERO(&rd);
	  FD_SET(hand_fd,&rd);
	  while(FD_ISSET(hand_fd,&rd))
	  {
		if(select(hand_fd+1,&rd,NULL,NULL,NULL) < 0)
		  perror("select error!\n");
		else
		{
		  while((nread = read(hand_fd,buff,sizeof(buff)))>0)
		  {  
			//printf("nread = %d,%s\n",nread,buff);
			buff[nread] = '\0';
			strcat(hand_message, buff);
			if(buff[nread-1] == '#')
			{
				printf("hmsg:%s\n",hand_message);
							
				//判断帧头，并验证
				char delim[] = ":";
				char *s = strdup(hand_message);
				if(strcmp(strsep(&s, delim),"ULM") == 0)		//正常msg
				{
					memset(Ulm_msg, 0, 32);
					sprintf(Ulm_msg,"����ǰ���ϰ��%smm",strsep(&s, delim));
					printf("\n%s\n", Ulm_msg);
				}				
				memset(hand_message,0,sizeof(hand_message));
			}
		  }
		}
	  }
	}
			
	close(hand_fd);
	pthread_exit(NULL);

}

/*
 *启动连接上hand
 *将hand信息解析成字符串存在command_arg�?
 *
 */
int start_hand( void  )
{
	hand_fd = open(HAND_DEV, O_RDWR|O_NOCTTY) ;

	uart_init(hand_fd);

	if( fcntl(hand_fd, F_SETFL, FNDELAY) < 0)
	{
		perror("fcntl hand_fd fail\n");
		exit(1);
	}
	

	tcdrain(hand_fd);
	tcflush(hand_fd,TCIOFLUSH);

	if( (pthread_create(&hand_read_t , NULL , hand_read_pthread , NULL )) != 0)
	{
		perror("Create the hand_read_pthread fail");
		exit(1);
	}
	
	return 0;

}