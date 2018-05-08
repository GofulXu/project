
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#ifdef _PRECISION_SIOCGSTAMP
#include <sys/ioctl.h>
#endif
#include "ntpclient.h"

#define USEC(x) ( ( (x) >> 12 ) - 759 * ( ( ( (x) >> 10 ) + 32768 ) >> 16 ) )

//char server[3][32] = {"ntp1.aliyun.com","ntp.sjtu.edu.cn","ntp5.aliyun.com"};
char server[3][32] = {"s1a.time.edu.cn","ntp.sjtu.edu.cn","s2b.time.edu.cn"};

char wbuf[128] = {0};

int main(int argc, char *argv[]) {
    int usd;  
    short int udp_local_port=0;   
    int cycle_time=5;           
    int probe_count=1;            
	int fp = open("./admin.m", O_CREAT|O_WRONLY, 0X775);
	while(1)
	{
		for(int i = 0; i < 3; i++)
		{
			if ((usd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1)
				{perror ("socket");exit(1);}

			setup_receive(usd, INADDR_ANY, udp_local_port);

			setup_transmit(usd, server[i], NTP_PORT);
#ifndef GOEFUL_TEST
			primary_loop(usd, probe_count, cycle_time);
#else
			struct timeval *tv;
			if(!primary_loop(usd, probe_count, cycle_time, 0, tv))
			{
				tv->tv_usec = USEC(tv->tv_sec);
				struct tm * mt = gmtime(&tv->tv_sec);
				printf("\nntp sync time success\nnew time = %02d-%02d-%02d %02d:%02d:%02d\n", 1900+mt->tm_year,1+mt->tm_mon,mt->tm_mday,mt->tm_hour+8,mt->tm_min,mt->tm_sec);
				printf("\n");
				close(usd);
				break;
			}else
			{
				printf("ntp sync time error url:%s\n", server[i]);
			}
			if(i == 2)
			{
				memset(wbuf,0,sizeof(wbuf));
				sprintf(wbuf,"all url sync ntp time fail%s\n", __DATE__);
				printf("%s\n", wbuf);
				write(fp, wbuf, strlen(wbuf));
				sync();
			}
#endif
			close(usd);
		}
		sleep(5);
	}
	close(fp);
    return 0;
}
