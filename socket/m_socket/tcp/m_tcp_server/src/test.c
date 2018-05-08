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
#include "gftcp.h"



//#define SERVER_IP "192.168.71.139"

//#define SERVER_IP "10.50.204.108"
#define SERVER_IP "192.168.1.108"

#define MAX_SOCKET 5
fd_set readfd,writefd,exceptfd;

static int msockfd[MAX_SOCKET];
static int mwsockfd[MAX_SOCKET];

int create_buf(char *sendbuf, char cmd_type, char msg_type, char display_mode, char *recvbuf, int tsize);
void heart_send(char *buf, int size);
void *goeful_run(void *arg);
void *task_run(void *arg);
void *goeful_wrun(void *arg);
void *task_wrun(void *arg);


int create_buf(char *sendbuf, char cmd_type, char msg_type, char display_mode, char *recvbuf, int tsize)
{
	static time_t now;
	static short num, data_len,i;
	int check_num;
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

	sendbuf[10] = cmd_type;
	sendbuf[11] = 0;
	sendbuf[12] = msg_type;
	sendbuf[13] = 0;
	sendbuf[14] = display_mode;
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
void heart_wsend(char *buf, int size)
{
	for(int i = 0; i < MAX_SOCKET; i++)
	{
		if(mwsockfd[i] > 0)
		{
			gf_tcp_send(mwsockfd[i], buf, size);
		}
	}
}
void heart_send(char *buf, int size)
{
	for(int i = 0; i < MAX_SOCKET; i++)
	{
		if(msockfd[i] > 0)
		{
			gf_tcp_send(msockfd[i], buf, size);
		}
	}
}
void *goeful_wrun(void *arg)
{
	printf("%s:%d\n", __FUNCTION__, __LINE__);
	fflush(stdout);
	unsigned long from_ip;
	unsigned short from_port;
	int sockfd = gf_tcp_socket();
	//gf_tcp_bind(sockfd, inet_addr("10.50.204.108"),htons(8888));
	gf_tcp_bind(sockfd, inet_addr(SERVER_IP),htons(8888));
	gf_tcp_listen(sockfd, 5);
	printf("bind 8888 suc %s:%d\n", __FUNCTION__, __LINE__);
	while(1){
		if(gf_tcp_select(sockfd,&readfd,&writefd,&exceptfd,22000))
		{
			printf("%s:%d\n", __FUNCTION__, __LINE__);

			int clientfd = gf_tcp_accept(sockfd, &from_ip, &from_port);
			pthread_t id = 0;
			pthread_create(&id, NULL, task_wrun, (void*)&clientfd);
			pthread_detach(id);
			for(int i = 0; i < MAX_SOCKET; i++)
			{
				if(mwsockfd[i] == 0)
				{
					mwsockfd[i] = clientfd;
					break;
				}else if(i == MAX_SOCKET-1)
				{
					close(clientfd);
				}
			}
			//printf("ip-->%s:%s",from_ip,from_port);
		}
		usleep(1000);
	}

	pthread_exit(NULL);

}

void *goeful_run(void *arg)
{
	printf("%s:%d\n", __FUNCTION__, __LINE__);
	fflush(stdout);
	unsigned long from_ip;
	unsigned short from_port;
	int sockfd = gf_tcp_socket();
	//gf_tcp_bind(sockfd, inet_addr("10.50.204.108"),htons(9999));
	gf_tcp_bind(sockfd, inet_addr(SERVER_IP),htons(9999));
	gf_tcp_listen(sockfd, 5);
	printf("bind 9999 suc %s:%d\n", __FUNCTION__, __LINE__);
	while(1){
		if(gf_tcp_select(sockfd,&readfd,&writefd,&exceptfd,22000))
		{
			printf("%s:%d\n", __FUNCTION__, __LINE__);
			int clientfd = gf_tcp_accept(sockfd, &from_ip, &from_port);
			pthread_t id = 0;
			pthread_create(&id, NULL, task_run, (void*)&clientfd);
			pthread_detach(id);
			for(int i = 0; i < MAX_SOCKET; i++)
			{
				if(msockfd[i] == 0)
				{
					msockfd[i] = clientfd;
					break;
				}else if(i == MAX_SOCKET-1)
				{
					close(clientfd);
				}
			}
			//printf("ip-->%s:%s",from_ip,from_port);
		}
		usleep(1000);
	}

	pthread_exit(NULL);

}

void *task_wrun(void *arg)
{
	printf("%s:%d\n", __FUNCTION__, __LINE__);
	fflush(stdout);
	int sockfd = *((int*)arg);
	char test_buf[1024] = {0}; 
	char *sbuf;
	int wsize;
	static int cond;
	cond = 0;
while(1){
		if(gf_tcp_select(sockfd,&readfd,&writefd,&exceptfd,2200))
		{
			memset(test_buf,0,sizeof(test_buf));
			int rsize = read(sockfd, test_buf, sizeof(test_buf));//读取信息
			if(rsize < 0)
			{
				perror("recv error");
				break;
			}
			if(test_buf[0] == 's' && test_buf[3] == 't' && test_buf[6] == 'f')
			{
				printf("\tinput cmd text\n ");
				printf("as: s1:t1:f1:%%s...\n ");
				printf("recv window data:%s text:%s\n", test_buf, test_buf + 9);
				wsize = strlen(test_buf+9);
				sbuf = (char *)malloc((wsize+22)*sizeof(char));
				memset(sbuf, 0, wsize+22);
				create_buf(sbuf, test_buf[1]-'0', test_buf[4] - '0', test_buf[7] - '0', test_buf + 9,wsize);
				heart_send(sbuf,wsize+22);
				//heart_wsend(sbuf+20,wsize);
				heart_wsend(sbuf,wsize+22);
				printf("send data:%s\n", sbuf+20);
				free(sbuf);
				usleep(1000);
			}
			cond = 0;
		}	
		cond++;
		if(cond == 10)
			break;
	}
	close(sockfd);
	pthread_exit(NULL);

}
void *task_run(void *arg)
{
	printf("%s:%d\n", __FUNCTION__, __LINE__);
	fflush(stdout);
	int sockfd = *((int*)arg);
	char test_buf[1024] = {0}; 
	static int cond;
	cond = 0;
while(1){
		if(gf_tcp_select(sockfd,&readfd,&writefd,&exceptfd,2200))
		{
			memset(test_buf,0,sizeof(test_buf));
			int rsize = read(sockfd, test_buf, sizeof(test_buf));//读取信息
			if(rsize < 0)
			{
				perror("recv error");
				break;
			}
			if(test_buf[0] == 0x0A)
			{
				printf("recv the mps heart data\n");
				gf_tcp_send(sockfd, test_buf, rsize);
			}
		}	
		cond ++;
		if(cond == 10)
			break;
	}
	
	close(sockfd);
	pthread_exit(NULL);

}

int main(void)
{
	memset(msockfd,0,MAX_SOCKET);
	memset(mwsockfd,0,MAX_SOCKET);
	pthread_t id = 0;
	pthread_create(&id, NULL, goeful_run, NULL);
	pthread_detach(id);
	pthread_t wid = 0;
	pthread_create(&wid, NULL, goeful_wrun, NULL);
	pthread_detach(wid);

	getchar();
	sleep(1);
	for(int i = 0; i < MAX_SOCKET; i++)
	{
		if(msockfd[i] > 0)
		{
			gf_tcp_close(msockfd[i]);
		}
	}

	for(int i = 0; i < MAX_SOCKET; i++)
	{
		if(mwsockfd[i] > 0)
		{
			gf_tcp_close(mwsockfd[i]);
		}
	}
	return 0;
}
