#include "robot.h"
#include "stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
int main(int argc, char **argv[])
{
	int ro_fd = open("/dev/robot",O_RDWR);
	unsigned int buf[6] = {0};
	int i = 0;
	if(ro_fd<0)
	{
			printf("open lcd error\n");
			return -1;
	}
	while(1)
	{
		scanf("%d",&i);
		switch(i){
			case 0:ioctl(ro_fd,ULTRASONIC_FUNCTION,0);read(ro_fd,buf,sizeof(buf));break;
			case 1:ioctl(ro_fd,INFRARED_FUNCTION,0);read(ro_fd,buf,sizeof(buf));break;
			case 2:ioctl(ro_fd,GO_RUN,0);break;
			case 3:ioctl(ro_fd,GO_BACK,0);break;
			case 4:ioctl(ro_fd,TURN_LEFT,0);break;
			case 5:ioctl(ro_fd,TURN_RIGHT,0);break;
			case 6:ioctl(ro_fd,GO_STOP,0);break;
			case 7:ioctl(ro_fd,LN298_ON_FUNCTION,0);break;
			case 8:ioctl(ro_fd,LN298_OFF_FUNCTION,0);break;
			case 10:ioctl(ro_fd,HAND_OUT0,1000);break;
			case 11:ioctl(ro_fd,HAND_OUT0,1500);break;
			case 12:ioctl(ro_fd,HAND_OUT0,2000);break;
			case 13:ioctl(ro_fd,HAND_OFF_FUNCTION,2000);break;
			
			case 9:return 0;
		}
		
		for(i = 0; i < 6;i++)
		{
			printf("buf%d:%d\n",i,buf[i]);
		}
	}
	
	
	close(ro_fd);
	return 0;
}