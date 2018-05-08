#include "uart.h"

/*设置串口1波特率为115200*/
void uart_init(int uart_fd)
{	
	struct termios termios_new;
	bzero( &termios_new, sizeof(termios_new));
	/*原始模式*/
	cfmakeraw(&termios_new);

	/*波特率为115200*/
	termios_new.c_cflag=(B115200);
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