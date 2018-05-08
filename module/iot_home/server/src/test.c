#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "gftcp.h"

#include "parse.h"
#include "cJSON.h"

#define SERVER_PORT 9999
//#define SERVER_IP "192.168.0.108"

#define MAX_SOCKET 200
fd_set readfd,writefd,exceptfd;
int msockfd[MAX_SOCKET];
Login_msg *m_dev;
cJSON *head;
static int isclose = 0;
void *goeful_run(void *arg);
void *task_run(void *arg);

void *goeful_run(void *arg)
{
	unsigned long from_ip;
	unsigned short from_port;
	int sockfd = gf_tcp_socket();
	int ret;
	ret = gf_tcp_bind(sockfd, 0, htons(SERVER_PORT));
	ret = gf_tcp_listen(sockfd, 5);
	if(ret == 0)
		printf("bind %d suc %s:%d\n", SERVER_PORT, __FUNCTION__, __LINE__);
	while(1){
		if(gf_tcp_select(sockfd,&readfd,&writefd,&exceptfd,2000))
		{
			printf("%s:%d\n", __FUNCTION__, __LINE__);
			int clientfd = gf_tcp_accept(sockfd, &from_ip, &from_port);
			printf("accept suc url:%d\n", from_port);
			for(int i = 0; i < MAX_SOCKET; i++)
			{
				if(msockfd[i] == 0)
				{
					pthread_t id = 0;
					pthread_create(&id, NULL, task_run, (void*)&clientfd);
					pthread_detach(id);
					msockfd[i] = clientfd;
					break;
				}else if(i == MAX_SOCKET-1)
				{
					printf("MAX_SOCKET_CLIENT %d\n", i);
					close(clientfd);
				}
			}
			//printf("ip-->%s:%s",from_ip,from_port);
		}
		if(isclose)
			break;
	}
	gf_tcp_close(sockfd);
	printf("%s return\n", __FUNCTION__);
	pthread_exit(NULL);

}


int msg_proc(Login_msg *recv_msg)
{
	Login_msg *tmp = NULL;
	Login_msg *head = NULL;
	printfmsg(recv_msg);
	HASH_FIND_INT(m_dev, &recv_msg->userid, tmp);
	if(NULL != tmp && LOGIN_STATUS != recv_msg->cmdid)
	{
		if(tmp->sockfd != recv_msg->sockfd)
		{
			gf_tcp_close(tmp->sockfd);
			tmp->sockfd = recv_msg->sockfd;
		}
		if(tmp->userid == recv_msg->userid && 0 == strcmp(tmp->password, recv_msg->password))
		{
			tmp->dev_status = 1;
			HASH_FIND_INT(m_dev, &recv_msg->devid, head);
			if(NULL != head)
			{
				printf("dev status:%d\tsockfd:%d\n", head->dev_status, head->sockfd);
				if(head->dev_status == 1 && head->sockfd > 0)
				{
					if(0 > gf_tcp_send(head->sockfd, recv_msg->cmdmsg, 1 + strlen(recv_msg->cmdmsg)))
					{
						close(head->sockfd);
						head->sockfd = -1;
						head->dev_status = 0;
						printf("send cmd error dev unline\n");
					}
				}
				else
					printf("dev unline\n");
			}else
			{
				printf("dev unable\n");
			}
			return 0;

		}else
		{
			printf("userid or passwd error\n");
			return -1;
		}
	}else if(LOGIN_STATUS == recv_msg->cmdid)
	{
		HASH_FIND_INT(m_dev, &recv_msg->userid, head);
		if(NULL != head)
		{
			if(head->userid != recv_msg->userid && 0 != strcmp(head->password, recv_msg->password))
			{
				printf("userid:%d able login error\n", head->userid);
				return -1;
			}
			if(head->sockfd != recv_msg->sockfd)
			{
				gf_tcp_close(head->sockfd);
				head->sockfd = recv_msg->sockfd;
				head->dev_status = 1;
				printf("userid:%d login success\n", head->userid);
				return 0;
			}
		}
		head = (Login_msg *)malloc(sizeof(Login_msg));
		memcpy(head, recv_msg, sizeof(Login_msg));
		head->dev_status = 1;
		HASH_ADD_INT(m_dev, userid, head);
		printf("userid:%d login create success\n", head->userid);
	}else
	{
		return -1;
	}
	return 0;

}

void *task_run(void *arg)
{
	int sockfd = *((int*)arg);
	char test_buf[1024] = {0}; 
	int cond;
	cond = 0;
	printf("%s\n", __FUNCTION__);
while(1){
		if(0 < gf_tcp_select(sockfd,&readfd,NULL,&exceptfd,1000))
		{
			memset(test_buf,0,sizeof(test_buf));
			int rsize = gf_tcp_recv(sockfd, test_buf, sizeof(test_buf));//读取信息
			if(rsize <= 0)
			{
				perror("recv size <= 0 error");
				break;
			}
			//printf("selcet success\t recv size:%d\n", rsize);
			head = cJSON_Parse(test_buf);
			if(NULL != head)
			{
				Login_msg *recv_msg = malloc(sizeof(Login_msg));
				if(0 == doit_login(head, recv_msg))
				{
					printf("doit_login success\n");
					recv_msg->sockfd = sockfd;
					if(0 > msg_proc(recv_msg))
						break;
				}
				free(recv_msg);
				recv_msg = NULL;
			
			}
			cond = 0;
		}
		cond++;
		if(cond == 10 || isclose)
			break;
	}
	for(int i = 0; i < MAX_SOCKET; i++)
	{
		if(msockfd[i] == sockfd)
		{
			msockfd[i] = 0;
		}
	}
	if(sockfd)
	{
		close(sockfd);
		printf("sockfd close %d return\n", sockfd);
	}
	printf("%s return\n", __FUNCTION__);
	pthread_exit(NULL);

}

int main(void)
{
	memset(msockfd,0,MAX_SOCKET);
	pthread_t id = 0;
	pthread_create(&id, NULL, goeful_run, NULL);
	pthread_detach(id);

	while('q' != getchar());

	isclose = 1;
	for(int i = 0; i < MAX_SOCKET; i++)
	{
		if(msockfd[i] > 0)
		{
			gf_tcp_close(msockfd[i]);
		}
	}

	sleep(2);
	return 0;
}
