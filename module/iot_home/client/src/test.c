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
int main(void)
{
	char *CC;
	int ret;
	pthread_t hid = 0;
	int sockfd = gf_tcp_socket();
	///gf_tcp_bind(sockfd, 0,htons(8686));
	ret = gf_tcp_connect(sockfd, inet_addr(SERVER_IP), htons(SERVER_PORT));
	if(ret != 0)
	{
		gf_tcp_close(sockfd);
		return 0;
	}
	printf("connect url:%s%d success\n", SERVER_IP, SERVER_PORT);
//	pthread_create(&hid, NULL, goeful_heartrun, (void *)&sockfd);
//	pthread_detach(hid);

	Login_msg m;
	cJSON *root = cJSON_CreateObject();
	strcpy(m.url, "tcp:192.168.1.108:9999");
	m.connect_type = 2;
	m.devtype = 246;
	strcpy(m.username, "goeful");
	m.devid = 2;
	m.userid = 601;
	strcpy(m.password, "goeful123..");
	m.cmdid = LOGIN_STATUS;
	//m.cmdid = GET_DEV_VALUE;
	strcpy(m.cmdmsg, "login");
	create_json_login(root, &m);
	CC = cJSON_PrintUnformatted(root);
	ret = gf_tcp_send(sockfd, CC, strlen(CC)+1);
	if(0 > ret)
		printf("send cmd login error\n");
	free(CC);
	cJSON_Delete(root);
	int i = 0;
	getchar();
#if 1
	while(1)
	{
		i++;
		if(i == 7)
			break;
		m.cmdid = GET_DEV_VALUE;
		m.devid = i;
		strcpy(m.cmdmsg, "temp");
		root = cJSON_CreateObject();
		create_json_login(root, &m);
		CC = cJSON_PrintUnformatted(root);
		ret = gf_tcp_send(sockfd, CC, strlen(CC)+1);
		free(CC);
		cJSON_Delete(root);
		if(0 < ret)
		{
			printf("send cmd success\n%s\n", CC);
			usleep(500000);
		}
		else
			break;
		usleep(500000);
#endif
	}
	gf_tcp_close(sockfd);
	isclose = 1;
	sleep(3);
	return 0;
}
