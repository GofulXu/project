#include "gfapi_linux.h"
#include "gfmem.h"
#include "gfthrd.h"
#include "gfsignal.h"
#include "gflog.h"
#include "gftype.h"
#include "gfmutex.h"
#include "gftcpclient.h"
#include "cJSON.h"
#include "parse.h"
#include "iot_define.h"


#define SERVER_IP "192.168.1.108"
#define SERVER_PORT 9999
#define HEARTCLIENT_TIME			4000
#define LOGIN_STATUS_NULL			0
#define LOGIN_STATUS_LOGIN			1
#define LOGIN_STATUS_CONNECTERROR	2
#define LOGIN_STATUS_SOCKERROR		3
#define M_VIDEOMSG_PREDEFINED_SIZE 5
#define M_SCROLLMSG_PREDEFINED_SIZE 5

static STcpClient m_tcpclient;
static HANDLE m_heartbeat_thrd = NULL;
static unsigned int m_thrd_tick = 0,m_heartbeat_tick = 0,m_respond_tick = 0;
static int m_heartbeat_response_limit = 0;
static int m_login_status = LOGIN_STATUS_NULL;


char *create_msg(m_cmdid_t cmdid, char *cmdmsg)
{
	char *CC = NULL;
	Login_msg m;
	cJSON *root = cJSON_CreateObject();
	strcpy(m.url, "tcp:192.168.1.108:9999");
	m.connect_type = MTYPE_TCP;
	m.devtype = M_DEVTYPE;
	strcpy(m.username, "goeful");
	m.devid = M_DEVID;
	m.userid = M_USERID;
	strcpy(m.password, "goeful123..");
	m.cmdid = cmdid;
	strcpy(m.cmdmsg, cmdmsg);
	create_json_login(root, &m);
	CC = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	return CC;
}

//发送心跳包
static int msgclient_sendmsg(STcpClient *tmp_client, m_cmdid_t cmdid, char *cmdmsg)
{
	char *CC = create_msg(cmdid, cmdmsg);
	int ret = gf_tcpclient_send(tmp_client, CC, strlen(CC)+1, 2000);
	free(CC);
	return ret;
}

//消息下发处理
static bool msgclient_onpacket(SOCKET socket, uint32_t ip, unsigned short port, char *buf, int size, uint32_t lParam )
{
	printf("size:%d recv:%s\n", size, buf);
	m_respond_tick = gf_thrd_get_tick();
	gf_thrd_delay(1000);

	return true;
}


static bool msgclient_onerror( uint32_t wparam, uint32_t lParam)
{
	fflush(stdout);
	gf_thrd_delay(1000);
	return true;
}


static bool msgclient_heartbeat_onerror( uint32_t wparam, uint32_t lparam )
{
#if 1
	fflush(stdout);
	if(true == gf_tcpclient_isinit(&m_tcpclient))
		gf_tcpclient_exit(&m_tcpclient);
#endif
	printf("test %s:%d\n", __FUNCTION__, __LINE__);
	return true;
}

//msgclient 开启心跳服务
static bool msgclient_heartbeat( uint32_t wparam, uint32_t lparam )
{

	if((m_login_status == LOGIN_STATUS_NULL || m_login_status == LOGIN_STATUS_CONNECTERROR || m_login_status == LOGIN_STATUS_SOCKERROR))
	{
		if(gf_tcpclient_init(&m_tcpclient,inet_addr(SERVER_IP),htons(SERVER_PORT),(PPacketHandler)msgclient_onpacket,msgclient_onerror,(uint32_t)0) )
		{
			m_tcpclient.check_entire = false;
        	printf("Connect msg server(%s:%d) success\n", SERVER_IP,SERVER_PORT);
			m_respond_tick = gf_thrd_get_tick(); 

			if(msgclient_sendmsg(&m_tcpclient, CMD_LOGIN, "login") > 0)
			{
        		printf("Send msg login to %s suc\n", SERVER_IP);
				m_login_status = LOGIN_STATUS_LOGIN;
			}
			else
			{
        		printf("Send msg login to %s fail\n", SERVER_IP);
				gf_tcpclient_exit(&m_tcpclient);
			
			}
		//	return true;

			
		}else
		{
			gf_thrd_delay(5000);
        	printf("Connect msg server(%s:%d) error\n", SERVER_IP, SERVER_PORT);
		}
        	//printf("Connect msg server(%s:%d) error", m_param.serverip,m_param.serverport);
			
	}
	if(m_login_status == LOGIN_STATUS_LOGIN)
	{
		if(gf_thrd_get_tick() - m_heartbeat_tick >= HEARTCLIENT_TIME)
		{
			if(msgclient_sendmsg(&m_tcpclient, CMD_HEART, "heart") < 0)
            {
        		printf("msg send heart command error\n");
                m_login_status = LOGIN_STATUS_SOCKERROR;
				if(true == gf_tcpclient_isinit(&m_tcpclient))
					gf_tcpclient_exit(&m_tcpclient);
			}
			m_heartbeat_tick = gf_thrd_get_tick();
		}

		/*超过20s没有收到心跳回应重新连接服务器*/
		if(gf_thrd_get_tick() - m_respond_tick >= m_heartbeat_response_limit)
		{
        	printf("More than %ds no receive response from msgserver,disconnect it\n", m_heartbeat_response_limit);
#if 1
			if(true == gf_tcpclient_isinit(&m_tcpclient))
				gf_tcpclient_exit(&m_tcpclient);
			m_login_status = LOGIN_STATUS_CONNECTERROR;
#endif
		}
	}

	/*线程空转时作下延时*/
	gf_thrd_delay(100);
	
	return true;

}

void gf_msgclient_init()
{
    m_heartbeat_response_limit = 10*1000;
	if(!m_heartbeat_thrd)
		m_heartbeat_thrd = gf_thrd_open( "msgHeartBeat", 80, 0, 4*1024, msgclient_heartbeat, msgclient_heartbeat_onerror, 0, 0);

	if(m_heartbeat_thrd)
		gf_thrd_resume(m_heartbeat_thrd);

	m_thrd_tick = m_heartbeat_tick = gf_thrd_get_tick();
	
}

int main(int argc, char *argv[])
{
	gf_msgclient_init();
	while('q' != getchar());
	gf_thrd_close(m_heartbeat_thrd, 1000);
	
	return 0;
}
