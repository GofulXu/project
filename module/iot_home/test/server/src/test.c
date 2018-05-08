#include "gfapi_linux.h"
#include "gfmem.h"
#include "gfthrd.h"
#include "gfsignal.h"
#include "gflog.h"
#include "gfmutex.h"
#include "gftcpserver.h"
#include "gftcp.h"
#include "parse.h"
#include "cJSON.h"
#include "iot_define.h"


#define LOGIN_STATUS_NULL			0
#define LOGIN_STATUS_LOGIN			1
#define LOGIN_STATUS_CONNECTERROR	2
#define LOGIN_STATUS_SOCKERROR		3
#define M_VIDEOMSG_PREDEFINED_SIZE 5
#define M_SCROLLMSG_PREDEFINED_SIZE 5
STcpServer *Sockserver = NULL;

Login_msg *m_dev;
int msg_proc(Login_msg *recv_msg)
{
	Login_msg *tmp = NULL;
	Login_msg *head = NULL;
	HASH_FIND_INT(m_dev, &recv_msg->userid, tmp);
	if(NULL != tmp && CMD_GET == recv_msg->cmdid)
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
	}else if(NULL != tmp && CMD_HEART == recv_msg->cmdid)
	{
		printf("heart\n");
		return 0;
	}else if(CMD_LOGIN == recv_msg->cmdid)
	{
		if(NULL != tmp)
		{
			if(tmp->userid != recv_msg->userid && 0 != strcmp(tmp->password, recv_msg->password))
			{
				printf("userid:%d unable login error\n", tmp->userid);
				return -1;
			}
			if(tmp->sockfd != recv_msg->sockfd && tmp->dev_status == 1)
			{
			//	gf_tcpserver_freeclient(tmp->sockfd);
			//	tmp->sockfd = recv_msg->sockfd;
				tmp->dev_status = 1;
				printf("userid:%d login success\n", head->userid);
				return 0;
			}else
			{
				tmp->sockfd = recv_msg->sockfd;
				tmp->dev_status = 1;
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
		printfmsg(recv_msg);
		return -1;
	}
	return 0;

}


//消息下发处理
static bool msgclient_onpacket(HANDLE Client, unsigned long ip, unsigned short port, char *buf, int size, unsigned long lParam )
{
#if 0
	Login_msg *recv_msg = NULL;
	STcpClient *m_Client = (STcpClient *)Client;
	cJSON *head = cJSON_Parse(buf);
	if(NULL != head)
	{
		recv_msg = malloc(sizeof(Login_msg));
		memset(recv_msg, 0, sizeof(Login_msg));
		if(0 == doit_login(head, recv_msg))
		{
#ifdef DEBUG
			printf("doit_login success\n");
#endif
			recv_msg->sockfd = m_Client->socket;
			if(0 > msg_proc(recv_msg))
			{
				goto ERROR;
			}
		}
	}else
		goto ERROR;

#ifdef DEBUG
	printf("%s:%d check suc size:%d recv:%s\n", __FUNCTION__, __LINE__, size, buf);
#endif
	if(NULL != recv_msg)
	{
		free(recv_msg);
		recv_msg = NULL;
	}
	if(NULL != head)
	{
		cJSON_Delete(head);
		head = NULL;
	}
	return true;

ERROR:
	printf("%s:%d check error size:%d recv:%s\n", __FUNCTION__, __LINE__, size, buf);
	if(NULL != recv_msg)
	{
		free(recv_msg);
		recv_msg = NULL;
	}
	if(NULL != head)
	{
		cJSON_Delete(head);
		head = NULL;
	}
	return false;
#endif
	gf_thrd_delay(1000);
	return true;
}


static bool msgclient_onerror(HANDLE Client, unsigned long ip, unsigned short port, unsigned long lParam)
{
	gf_thrd_delay(1000);
	fflush(stdout);
	return true;
}

bool heart_check(uint64_t c, uint64_t b)
{
	gf_thrd_delay(1000);

	return true;
}


bool heart_send(uint64_t c, uint64_t b)
{
	gf_tcpserverheart_send(Sockserver, "test", strlen("test")+1, 2000);
	gf_thrd_delay(4000);
	return true;
}
int main(int argc, char *argv[])
{

	Sockserver = gf_tcpserver_init(0, htons(9999), msgclient_onpacket, msgclient_onerror, 0);
	if(Sockserver == NULL)
		return 0;
	HANDLE m_heart  = gf_thrd_open("heart_check", DEFAULT_PRIORIOTY, GF_SCHED_NORMAL, DEFAULT_STACK_SIZE, heart_send, NULL, 0, 0);

	if(NULL != m_heart){
		gf_thrd_resume(m_heart);	//开始运行线程
	}
	
	while('q' != getchar());
	gf_thrd_close(m_heart, 2000);
	gf_tcpserver_exit(Sockserver);
	gf_thrd_delay(1000);
	return 0;
}
