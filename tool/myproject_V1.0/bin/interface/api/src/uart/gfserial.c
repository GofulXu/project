#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "gfserial.h"
#include "gflog.h"

#define USE_NON_ICANON

typedef struct {
	int fd;
	char * dev;
}gf_serial_t;
#define MAX_SERIAL_NUM 3
gf_serial_t m_sList[MAX_SERIAL_NUM] = {
	{-1, NULL},
	{-1, NULL},
	{-1, NULL}
};

int32_t gf_serial_open(const char *dev)
{
	int fd, err, i;
	if( dev == NULL )
	{
		GFSERIAL_LOG_DEBUG("%s: Invalid parameter\n",__func__);
		return -1;
	}
	for( i=0; i<MAX_SERIAL_NUM; i++)
	{
		if( m_sList[i].fd >= 0 && m_sList[i].dev && strcmp(m_sList[i].dev, dev) == 0 )
		{
			GFSERIAL_LOG_DEBUG("[%s:%d] %s alread opened\n",__func__,__LINE__,dev);
			return m_sList[i].fd;
		}
	}
	for( i=0; i<MAX_SERIAL_NUM; i++)
	{
		if( m_sList[i].fd == -1 )
			break;
	}

	fd = open(dev, O_NOCTTY|O_NONBLOCK|O_RDWR);
	if( fd == -1 )
	{
		err = errno;
		GFSERIAL_LOG_DEBUG("open %s failed: %s\n",dev, strerror(err));
		return -1;
	}

	if( i == MAX_SERIAL_NUM )
	{
		i = 0;
		close(m_sList[i].fd);
		if( m_sList[i].dev )
			free(m_sList[i].dev);
		m_sList[i].dev = NULL;
	}
	m_sList[i].fd = fd;
	m_sList[i].dev = (char *)malloc(strlen(dev)+1);
	if( m_sList[i].dev )
		sprintf(m_sList[i].dev, "%s", dev);

	GFSERIAL_LOG_DEBUG("[%s:%d] fd:%d\n",__func__,__LINE__, fd);
	return fd;
}

void gf_serial_close(int fd)
{
	int i;
	GFSERIAL_LOG_DEBUG("[%s:%d] fd:%d\n",__func__,__LINE__, fd);
	close(fd);
	for( i=0; i<MAX_SERIAL_NUM; i++)
	{
		if( m_sList[i].fd == fd )
		{
			m_sList[i].fd = -1;
			if( m_sList[i].dev )
				free( m_sList[i].dev );
			m_sList[i].dev = NULL;
			break;
		}
	}
}

int32_t gf_serial_read(int fd, unsigned char *buf, int len, int timeout)
{
	int rsize, rlen=0, err;
	struct timeval tv;
	fd_set readfds;
	FD_ZERO(&readfds);

	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout%1000)*1000;
	GFSERIAL_LOG_DEBUG("[%s:%d] fd:%d\n",__func__,__LINE__, fd);

select_again:
	FD_SET(fd, &readfds);
	err = select( fd+1, &readfds, NULL, NULL, (tv.tv_sec>0 || tv.tv_usec>0)? (&tv):NULL );
	if( err == -1 )
	{
		GFSERIAL_LOG_DEBUG("select error: %s\n", strerror(errno));
		return -1;
	}
	else if( err )
	{
		if( FD_ISSET(fd, &readfds) )
		{
			if( (rsize = read( fd, buf+rlen, len-rlen)) > 0 )
			{
				rlen += rsize;
			}
		}
		else
		{
			GFSERIAL_LOG_DEBUG("[%s:%d] select again\n",__func__,__LINE__);
			goto select_again;
		}
	}
	else
		GFSERIAL_LOG_DEBUG("[%s:%d] select timeout\n",__func__,__LINE__);
	buf[rlen] = 0;
	GFSERIAL_LOG_DEBUG("[%s:%d] read(%d bytes): [%s]\n",__func__,__LINE__, rlen, buf);
	return rlen;
}

int32_t gf_serial_write(int fd, unsigned char *buf, int len, int timeout)
{
	int wsize = 0, err;
	struct timeval tv;
	fd_set writefds;
	FD_ZERO(&writefds);

	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout%1000)*1000;

	GFSERIAL_LOG_DEBUG("[%s:%d] fd:%d\n",__func__,__LINE__, fd);
write_again:
	FD_SET(fd, &writefds);
	err = select( fd+1, NULL, &writefds, NULL, (tv.tv_sec>0 || tv.tv_usec>0)? (&tv):NULL );
	if( err == -1 )
	{
		GFSERIAL_LOG_DEBUG("select error: %s\n", strerror(errno));
		return -1;
	}
	else if( err )
	{
		if( FD_ISSET(fd, &writefds) )
		{
			wsize = write( fd, buf, len);
		}
		else
			goto write_again;
	}
	return wsize;
}

static int speed_arr[] = { B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
	B38400, B19200, B9600, B4800, B2400, B1200, B300, };
static int name_arr[] = {115200, 38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,
	19200,  9600, 4800, 2400, 1200,  300, };

/**
 *	@brief 设置串口参数
 *  
 *	@param fd 串口句柄
 *	@param speed 波特率
 *	@param databits 数据位数
 *  @param stopbits 停止位位数
 *	@param parity 模式
 *	
 *	@return 设置成功返回0,失败返回-1
 */
int gf_serial_set_parity(int fd,int speed,int databits,int stopbits,int parity)
{
	int i;
	int is_speed_true = 0;
	struct termios options; 
	GFSERIAL_LOG_DEBUG("[%s:%d] fd:%d\n",__func__,__LINE__, fd);
	tcflush(fd, TCIOFLUSH);
	if  ( tcgetattr( (int)fd, &options )  !=  0)
	{ 
		GFSERIAL_LOG_DEBUG("tcgetattr failed: %s\n",strerror(errno));
		return -1;  
	}
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {
		if  (speed == name_arr[i]) {
			is_speed_true = 1;
			break;
		}
	}
    if(is_speed_true == 1)
	{
		cfsetispeed(&options, speed_arr[i]);
		cfsetospeed(&options, speed_arr[i]);     
	}
	else
	{
		GFSERIAL_LOG_DEBUG("set speed failed: %s\n",strerror(errno));
		return -1;  
	}
	options.c_iflag = 0;
	options.c_oflag = 0;

	options.c_cflag &= ~CSIZE; 
	switch (databits) /*......*/
	{   
		case 5:     
			options.c_cflag |= CS5; 
			break;
		case 6:     
			options.c_cflag |= CS6; 
			break;
		case 7:     
			options.c_cflag |= CS7; 
			break;
		case 8:     
			options.c_cflag |= CS8;
			break;   
		default:    
			GFSERIAL_LOG_DEBUG("Unsupported data sizen"); 
			return -1;  
	}
	switch (parity) 
	{   
		case 'n':
		case 'N':    
			options.c_cflag &= ~PARENB;		/* Clear parity enable */
			options.c_iflag &= ~INPCK;		/* Disable parity checking */ 
			break;  
		case 'o':   
		case 'O':     
			options.c_cflag |= PARENB;		/* Enable parity */    
			options.c_cflag |= PARODD;		/* Parity for input and output is odd */
			options.c_iflag |= INPCK;		/* Enable input parity checking */ 
			break;  
		case 'e':  
		case 'E':   
			options.c_cflag |= PARENB;		/* Enable parity */    
			options.c_cflag &= ~PARODD;		/* Parity for input and output is even */ 
			options.c_iflag |= INPCK;		/* Enable input parity checking */
			break;
		case 'S': 
		case 's':  /*as no parity*/   
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;  
		default:   
			GFSERIAL_LOG_DEBUG("Unsupported parityn");    
			return -1;  
	}
	/* .....*/  
	switch (stopbits)
	{   
		case 1:    
			options.c_cflag &= ~CSTOPB;  
			break;  
		case 2:    
			options.c_cflag |= CSTOPB;  
			break;
		default:    
			GFSERIAL_LOG_DEBUG("Unsupported stop bitsn");  
			return -1; 
	}

    //设置校验位
	switch(parity)
	{
		case 'n'://无奇偶校验
		case 'N':
			options.c_cflag &= ~PARENB;
			options.c_iflag &= ~INPCK;
			break;
		case 'o'://奇校验
		case 'O':
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK;
			break;
		case 'e'://偶校验
		case 'E':
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
			break;
		case 's'://空格
		case 'S':
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default ://默认无奇偶校验
			GFSERIAL_LOG_DEBUG("Unsupported parity bitsn");  
			return -1; 
	}

#ifdef USE_NON_ICANON
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
#else
	options.c_lflag  &= ~( ECHO | ECHOE | ISIG);  /*Input*/
#endif
	options.c_oflag  &= ~OPOST;   /*Output*/

	tcflush( (int)fd, TCIFLUSH );
#ifdef USE_NON_ICANON
	options.c_cc[VTIME] = 10; /* ....15 seconds*/   
	options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
#endif
	if (tcsetattr( (int)fd, TCSANOW, &options ) != 0)   
	{ 
		GFSERIAL_LOG_DEBUG("tcgetattr failed: %s\n",strerror(errno));
		return -1;  
	}

	//fcntl((int)fd, F_SETFL, O_NDELAY); 
	return 0;

}

