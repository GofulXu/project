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
#include <pthread.h>
#include "swtcp.h"
fd_set readfd,writefd,exceptfd;

int create_buf(char *sendbuf, char *recvbuf, int tsize)
{
	static time_t now;
	static short num, data_len,i;
	int check_num;
	if(sendbuf == NULL)
		sendbuf = (char *)malloc((tsize+22)*sizeof(char));
	memset(sendbuf, 0, tsize+22);
	now = time(NULL);
	srand(now);//%sizeof(short);
	num = rand()%10000;
	sendbuf[0] = 0X0A;
	sendbuf[1] = (char)(num >> 8) ;
	sendbuf[2] = (char)((num << 8) >> 8);
	sendbuf[5] = 0X02;
	sendbuf[6] = 0X01;
	sendbuf[7] = 0X02;
	sendbuf[8] = 0X03;
	sendbuf[9] = 0X04;

	sendbuf[10] = 0;
	sendbuf[11] = 0;
	sendbuf[12] = 1;
	sendbuf[13] = 0;
	sendbuf[14] = 1;
	sendbuf[15] = 0;
	sendbuf[16] = 0;
	sendbuf[17] = 0;
	sendbuf[18] = tsize;
	sendbuf[19] = 0;
	memcpy(sendbuf+20,recvbuf,tsize);
	data_len = 1+4+tsize;
	sendbuf[3] = (char)(data_len >> 8);
	sendbuf[4] = (char)((data_len << 8) >> 8);
	check_num = 0;
	for(i = 1; i < tsize+22-2; i++)
	{
		check_num += sendbuf[i];
	}
	sendbuf[tsize+22-2] = (char)((check_num << 24) >> 24);
	sendbuf[tsize+22-1] = 0XA0;
	return tsize+22;
}

void *task_run(void *arg)
{
	int sockfd = *((int*)arg);
	char test_buf[1024] = {0}; 
while(1){
		if(sw_tcp_select(sockfd,&readfd,&writefd,&exceptfd,2200))
		{
			memset(test_buf,0,sizeof(test_buf));
			int rsize = read(sockfd, test_buf, sizeof(test_buf));//读取信息
			if(rsize < 0)
			{
				perror("recv error");
				break;
			}
			printf("buf:%s\n",test_buf);
			if(test_buf[0] == 0x0A)
			{
				sw_tcp_send(sockfd, test_buf, rsize);
			}
		}	
	}

	close(sockfd);
	pthread_exit(NULL);
}
int main(void)
{
	unsigned long from_ip;
	unsigned short from_port;
	int wsize;
	char textbuf[] = "goeful";
	char *sbuf;
	wsize = strlen(textbuf);
	sbuf = (char *)malloc((wsize+22)*sizeof(char));
	int sockfd = sw_tcp_socket();
	sw_tcp_bind(sockfd, inet_addr("192.168.101.108"),htons(9999));
	sw_tcp_listen(sockfd, 5);
	while(1){
		if(sw_tcp_select(sockfd,&readfd,&writefd,&exceptfd,22000))
		{
			int clientfd = sw_tcp_accept(sockfd, &from_ip, &from_port);
	//		pthread_t id = 0;
	//		pthread_create(&id, NULL, task_run, (void*)&clientfd);
	//		pthread_detach(id);

			memset(sbuf, 0, wsize+22);
			create_buf(sbuf, textbuf,wsize);
			sw_tcp_send(clientfd, sbuf, wsize+22);
			//printf("ip-->%s:%s",from_ip,from_port);
		}
		usleep(1000);
	}
	sw_tcp_close(sockfd);
	return 0;

}
