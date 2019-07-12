#include "gfapi.h"
#include "gfmutex.h"
#include "gfcom.h"      
#include "gfthrd.h"

//串口速率信息
static int speed_arr[] = { B38400, B115200, B9600, B4800, B2400, B1200, B300,
	    B38400, B19200, B9600, B4800, B2400, B1200, B300, };
static int name_arr[] = {38400,  115200,  9600,  4800,  2400,  1200,  300,
	    38400,  19200,  9600, 4800, 2400, 1200,  300, };

/**
 * @brief 设置串口速率
 */
static void set_speed(int fd, int speed)
{
  int   i;
  int   status;
  struct termios   Opt;
  tcgetattr(fd, &Opt);
  for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
   {
   	if  (speed == name_arr[i])
   	{
   	    tcflush(fd, TCIOFLUSH);
    	cfsetispeed(&Opt, speed_arr[i]);
    	cfsetospeed(&Opt, speed_arr[i]);
    	status = tcsetattr(fd, TCSANOW, &Opt);
    	if  (status != 0)
            perror("tcsetattr fd1");
     	return;
     	}
   tcflush(fd,TCIOFLUSH);
   }
}

/**
*@brief   设置串口数据位，停止位和效验位
*@param  fd     类型  int  打开的串口文件句柄*
*@param  databits 类型  int 数据位   取值 为 7 或者8*
*@param  stopbits 类型  int 停止位   取值为 1 或者2*
*@param  parity  类型  int  效验类型 取值为N,E,O,,S
*/
static int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;
 if  ( tcgetattr( fd,&options)  !=  0)
  {
  	perror("SetupSerial 1");
  	return(-1);
  }
  options.c_cflag &= ~CSIZE;
  //options.c_iflag &= ~(ICRNL | IXON);
  switch (databits) /*设置数据位数*/
  {
  	case 7:
  		options.c_cflag |= CS7;
  		break;
  	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr,"Unsupported data size\n");
		return (-1);
	}
  switch (parity)
  	{
  	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;   /* Clear parity enable */
		options.c_iflag &= ~INPCK;     /* Enable parity checking */
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);  /* 设置为奇效验*/ 
		options.c_iflag |= INPCK;             /* Disnable parity checking */
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;     /* Enable parity */
		options.c_cflag &= ~PARODD;   /* 转换为偶效验*/  
		options.c_iflag |= INPCK;       /* Disnable parity checking */
		break;
	case 'S':
	case 's':  /*as no parity*/
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr,"Unsupported parity\n");
		return (-1);
		}
  /* 设置停止位*/   
  switch (stopbits)
  	{
  	case 1:
  		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr,"Unsupported stop bits\n");
		return (-1);
	}
  /* Set input parity option */
  if (parity != 'n')
  		options.c_iflag |= INPCK;
    options.c_cc[VTIME] = 150; // 15 seconds
    options.c_cc[VMIN] = 0;

  tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */
  if (tcsetattr(fd,TCSANOW,&options) != 0)
  	{
  		perror("SetupSerial 3");
		return (-1);
	}
  return (0);
 }

/**
*@breif 打开串口
*/
static int OpenDev(char *Dev)
{
	int	fd = open( Dev, O_RDWR );         //| O_NOCTTY | O_NDELAY
	if (-1 == fd)
		{ /*设置数据位数*/
			perror("Can't Open Serial Port");
			return -1;
		}
	else
	    return fd;
}

static bool error_thrd(uint64_t wParam,uint64_t lParam)
{
	return true;
}
static bool recv_thrd(uint64_t wParam,uint64_t lParam)
{
	int nread = 0;
	ComClient_t *comClient = (ComClient_t *)wParam;

	char dev[128]="/dev/ttyUSB";
	//char dev[128]="/dev/ttyAMA";
	snprintf(dev + strlen(dev), sizeof(dev), "%d", comClient->uart_port);

    fd_set rfds;
    struct termios opt;
	struct timeval tv;

REOPEN: 
	printf("Init com device [%s] with rate [%d] timeout [%d]", dev, comClient->rate, comClient->timeout/1000);

	//打开串口
	comClient->socket = OpenDev(dev);

	//设置串口速率
	if (comClient->socket>0)
    	set_speed(comClient->socket,comClient->rate);
	else
	{
		printf( "Can't Open Serial Port!");
		gf_thrd_delay(1000);
		goto REOPEN;
	}
	//设置串口参数
  	if (set_Parity(comClient->socket,8,1,'N')== -1)
  	{
		close(comClient->socket);
		comClient->socket = -1;
    	printf( "Set Parity Error");
		gf_thrd_delay(1000);
		goto REOPEN;
  	}

    tcgetattr(comClient->socket,&opt);
    opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    opt.c_oflag &=~(INLCR|IGNCR|ICRNL);
    opt.c_oflag &=~(ONLCR|OCRNL);
    opt.c_iflag &=~(ICRNL|IXON);
    tcsetattr(comClient->socket, TCSANOW, &opt);
   	tcflush(comClient->socket, TCIOFLUSH);

    //读取串口数据
  	while(1)
  	{
        if(comClient->exit)
            return false;

		//超时没有收到数据时，通知上层
        if(gf_thrd_get_tick()- comClient->last_recv_time > comClient->timeout)
		{
			if(comClient->pComTimeoutHandler)
				comClient->pComTimeoutHandler(0);
			comClient->last_recv_time = gf_thrd_get_tick();	
		}

		memset(comClient->recv_buf, 0, sizeof(comClient->recv_buf));
        FD_ZERO(&rfds);
        FD_SET(comClient->socket, &rfds);
   	    tcflush(comClient->socket, TCIOFLUSH);

        tv.tv_sec = 2; 
        tv.tv_usec = 0;

        int activeCount = -1;
        if((activeCount = select(comClient->socket + 1, &rfds, NULL, NULL, &tv)) < 0)
        {
            continue;
        }
        else if(activeCount > 0 && FD_ISSET(comClient->socket, &rfds))
        {
            if((nread = read(comClient->socket,comClient->recv_buf,sizeof(comClient->recv_buf))) <= 0)
            {
				close(comClient->socket);
				comClient->socket = -1;
				printf( "Read data from serial port %s error reopen it", dev);
				gf_thrd_delay(500);
				goto REOPEN;
            }
        }
        else if(activeCount == 0)
        {
            continue;
        }

		if(comClient->reboot)
		{
			close(comClient->socket);
			comClient->socket = -1;
			gf_thrd_delay(500);
			comClient->reboot = false;
			goto REOPEN;
		}

		if(comClient->debug)
		{
			int i = 0;
			for(i = 0; i < nread; i++)
				printf("%02x ", comClient->recv_buf[i]);
			printf(" length = %d\n\n",nread);
		}

		if(comClient->pComDataHandler)
			comClient->pComDataHandler(comClient->recv_buf, nread);

        comClient->last_recv_time = gf_thrd_get_tick();
  	}
    close(comClient->socket);
    gf_thrd_delay(1000);
	return true;
}

int gf_com_set_reboot(ComClient_t *comClient)
{
	comClient->reboot = true;
	return 0;
}

bool gf_com_send_data(ComClient_t *comClient, unsigned char *data, int size)
{
	int rtn = write(comClient->socket, data, size); 
	if(rtn <= 0)
		printf("com write error\n");
	fsync(comClient->socket);

    return 0;
}

int gf_com_init(ComClient_t *comClient)
{
	comClient->tRecvThrd = gf_thrd_open("tComRecvThrd",100,0,4096, recv_thrd, error_thrd, (uint64_t)comClient, 0);
	if(comClient->tRecvThrd)
		gf_thrd_resume(comClient->tRecvThrd);

    comClient->last_recv_time = gf_thrd_get_tick();

	comClient->exit = false;
	return 0;
}

int gf_com_exit(ComClient_t *comClient)
{
	comClient->exit = true;

	if(comClient->socket > 0)
	{
		close(comClient->socket);	
		comClient->socket = -1;
	}

    if(comClient->tRecvThrd)
    {
        gf_thrd_close(comClient->tRecvThrd, 100);
        comClient->tRecvThrd = NULL;
    }

    return 0;
}
