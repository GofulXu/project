#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "ksa.h"
int main()
{
	int fd,id = 0;
	unsigned char ksa_val;

	fd = open("/dev/ksa_btn",O_RDWR);
	if(fd<0)
	{
		printf("open btn failed!\n");
		return -1;
	}
	
	scanf("%d",&id);
	switch(id)
	{
		case GPIO_ON:ioctl(fd,GPIO_ON,DF_UART3_TX);
			   read(fd,&ksa_val,sizeof(ksa_val));
			   printf("ksa_val = %#x\n",ksa_val);
			   break;
		case GPIO_OFF:ioctl(fd,GPIO_OFF,DF_UART3_RX);
			   read(fd,&ksa_val,sizeof(ksa_val));
			   printf("ksa_val = %#x\n",ksa_val);
			   break;

		default:break;
	}

	read(fd,&ksa_val,sizeof(ksa_val));
	printf("ksa_val = %#x\n",ksa_val);

	close(fd);
	return 0;
}

