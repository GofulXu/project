#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include "uart.h"

static int uart_fd;
static pthread_t uart_read_t;

int close_uart(void)
{
	return close(uart_fd);
}

int send_to_uart(char *buf, int size)
{
	if(uart_fd)
		return write(uart_fd, buf, size);
	else
		return -1;
}

static void *uart_read_pthread(void *fd_arg)
{
  int nread,i;
  char buff[16];
  char buf[1024];
  fd_set rd;
  
  //利用select函数来实现多个串口的读写
	while(1)
	{
	  if(uart_fd < 0)
		  break;
	  FD_ZERO(&rd);
	  FD_SET(uart_fd,&rd);
	  while(FD_ISSET(uart_fd,&rd))
	  {
		if(select(uart_fd+1,&rd,NULL,NULL,NULL) < 0)
		  perror("select error!\n");
		else
		{
		  while((nread = read(uart_fd,buff,sizeof(buff)))>0)
		  {  
			
			memset(buf,0,sizeof(buf));
			sprintf(buf, "recv data: ");
			for(i = 0; i < nread; i++)
			{
				sprintf(buf, "%s %x", buf, buff[i]);
			}
			printf("%s", buf);
			printf("nread = %d,%s\n",nread,buff);
			
		  }
		}
	  }
	}
	pthread_exit(NULL);
}



/*设置串口1波特率*/
static void uart_init(int uart_fd, int rate)
{	
	struct termios termios_new;
	bzero( &termios_new, sizeof(termios_new));
	/*原始模式*/
	cfmakeraw(&termios_new);

	/*波特率为115200*/
	switch(rate)
	{
		case 9600: termios_new.c_cflag=(B9600);break;
		case 115200: termios_new.c_cflag=(B115200);break;
		default: termios_new.c_cflag=(B115200);break;
	}
	termios_new.c_cflag |= CLOCAL | CREAD;
	
	/*8位数据位*/
	termios_new.c_cflag &= ~CSIZE;
	termios_new.c_cflag |= CS8;

	/*无奇偶校验位*/
	termios_new.c_cflag &= ~PARENB;

	/*1位停止位*/
	termios_new.c_cflag &= ~CSTOPB;
	/*清除串口缓冲区*/
	tcflush( uart_fd,TCIFLUSH);
	termios_new.c_cc[VTIME] = 0;
	termios_new.c_cc[VMIN] = 4;
	tcflush ( uart_fd, TCIFLUSH);
	/*串口设置使能*/
	tcsetattr( uart_fd ,TCSANOW,&termios_new);
}


int start_uart(char *dev, int rate, bool read_bool)
{
	if(dev)
		uart_fd = open(dev, O_RDWR|O_NOCTTY);
	else
		uart_fd = open(UART_DEV, O_RDWR|O_NOCTTY);
	if(uart_fd < 0)
	{
		perror("open uart__id error\n");	
		return -1;
	}
	uart_init(uart_fd, rate);

	if( fcntl(uart_fd, F_SETFL, FNDELAY) < 0)
	{
		perror("fcntl uart_fd fail\n");
		return -1;
	}
	tcdrain(uart_fd);
	tcflush(uart_fd,TCIOFLUSH);
	if(true == read_bool)
	{
		if( (pthread_create(&uart_read_t , NULL , uart_read_pthread , NULL )) != 0)
		{
			perror("Create the hand_read_pthread fail");
			return -1;
		}
	}
	return 0;
}


