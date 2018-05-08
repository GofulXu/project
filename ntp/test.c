
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
#include <errno.h>
#ifdef _PRECISION_SIOCGSTAMP
#include <sys/ioctl.h>
#endif
#include "ntpclient.h"

#define USEC(x) ( ( (x) >> 12 ) - 759 * ( ( ( (x) >> 10 ) + 32768 ) >> 16 ) )


int main(int argc, char *argv[]) {
    int usd;  
    short int udp_local_port=0;   
    int cycle_time=5;           
    int probe_count=1;            

    char *hostname=argv[1];

    if ((usd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1)
        {perror ("socket");exit(1);}

    setup_receive(usd, INADDR_ANY, udp_local_port);

    setup_transmit(usd, hostname, NTP_PORT);
#ifndef GOEFUL_TEST
    primary_loop(usd, probe_count, cycle_time);
#else
	struct timeval *tv;
	if(!primary_loop(usd, probe_count, cycle_time, 0, tv))
	{
		tv->tv_usec = USEC(tv->tv_sec);
		struct tm * mt = gmtime(&tv->tv_sec);
		printf("\nntp sync time success\nnew time = %02d-%02d-%02d %02d:%02d:%02d\n", 1900+mt->tm_year,1+mt->tm_mon,mt->tm_mday,mt->tm_hour+8,mt->tm_min,mt->tm_sec);
	}
#endif
    close(usd);
    return 0;
}
