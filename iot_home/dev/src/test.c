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

#include "parse.h"
#include "cJSON.h"

#define MID 2
#define SERVER_IP "192.168.0.108"
//#define SERVER_IP "45.76.132.152"
#define SERVER_PORT 9999
int isclose = 0;
void *goeful_heartrun(void *arg)
{
	int sockfd = *(int *)arg;
	int ret;
	while(1)
	{
		ret = gf_tcp_send(sockfd, "heart", 6);
		if(ret < 0 || isclose == 1)
			break;
		sleep(1);
	}	
	printf("%s return\n", __FUNCTION__);
	pthread_exit(NULL);

}
void *goeful_recvrun(void *arg)
{
	int ret;
	int sockfd = *(int *)arg;
	char recv[1024];
	while(1)
	{
		fd_set readfds;
		if(gf_tcp_select(sockfd, &readfds, NULL, NULL, 1000))
		{
			memset(recv, 0, sizeof(recv));
			ret = gf_tcp_recv(sockfd, recv, sizeof(recv));
			if(ret <= 0)
				break;
			else
				printf("recvdata:%s\n", recv);
		}
		if(isclose == 1)
			break;
	}
	printf("%s return\n", __FUNCTION__);
	pthread_exit(NULL);
}
int main(void)
{
	Login_msg m;
	int ret;
	char *CC;
	pthread_t rid = 0, wid = 0;
	int sockfd = gf_tcp_socket();
	//gf_tcp_bind(sockfd, 0,htons(8686));
	ret = gf_tcp_connect(sockfd, inet_addr(SERVER_IP), htons(SERVER_PORT));
	if(ret == 0)
	{
	//	pthread_create(&wid, NULL, goeful_heartrun, (void *)&sockfd);
	//	pthread_detach(wid);
		pthread_create(&rid, NULL, goeful_recvrun, (void *)&sockfd);
		pthread_detach(rid);
	}else
	{
		printf("connect url:%s:%d error\n", SERVER_IP, SERVER_PORT);
		return -1;
	}
	printf("connect url:%s:%d success\n", SERVER_IP, SERVER_PORT);
	cJSON *root = cJSON_CreateObject();
	strcpy(m.url, "tcp:192.168.1.108:9999");
	m.connect_type = 2;
	m.devtype = 123;
	strcpy(m.username, "dev240");
	m.devid = MID;
	m.userid = MID;
	strcpy(m.password, "aaaaa");
	m.cmdid = LOGIN_STATUS;
	//m.cmdid = GET_DEV_VALUE;
	strcpy(m.cmdmsg, "login");
	create_json_login(root, &m);
	CC = cJSON_PrintUnformatted(root);
	gf_tcp_send(sockfd, CC, strlen(CC)+1);
	free(CC);
	cJSON_Delete(root);
	while('q' != getchar());
	isclose = 1;
	gf_tcp_close(sockfd);
	sleep(4);
	return 0;
}
