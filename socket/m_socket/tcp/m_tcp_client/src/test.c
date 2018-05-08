#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "gftcp.h"
//fd_set readfd,writefd,exceptfd;

static void printf02hex(char *recv,int size)
{
	char log_buf[1024] = {0};
	char text[3] = {0};
	memset(log_buf, 0, sizeof(log_buf));
	memset(text, 0, sizeof(text));
	for(int i = 0; i < size; i++){
		snprintf(text," %02x", recv[i]);
		memcpy(log_buf+i*3,text,3);
	}
		//printf(" %02x", recv[i]);
	printf("recv: %s\n", log_buf);
#if 0
	static int i = 0;
	static char log_buf[1024] = {0};
	memset(log_buf, 0, sizeof(log_buf));
	for(i = 0; i < size; i++)
		sprintf(log_buf, "%s %02x", log_buf, recv[i]);
	printf("recv: %s\n", log_buf);
#endif
}


int main(void)
{
	int msockfd,rsize;
	char recvbuf[820];
	memset(recvbuf,0,sizeof(recvbuf));
	msockfd = gf_tcp_socket();
	gf_tcp_connect(msockfd, inet_addr("192.168.1.108"), htons(9999));
//	while(1)
	{
		rsize = recv(msockfd, recvbuf, 820, 0);
		printf("size:%d, %02x\n", rsize, recvbuf[819]);
		printf02hex(recvbuf, rsize);
	}
	gf_tcp_close(msockfd);
	return 0;

}
