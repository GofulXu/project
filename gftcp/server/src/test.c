#include "gfapi_linux.h"
#include "gfmem.h"
#include "gfthrd.h"
#include "gfsignal.h"
#include "gflog.h"
#include "gfmutex.h"
#include "gftcpserver.h"


#define LOGIN_STATUS_NULL			0
#define LOGIN_STATUS_LOGIN			1
#define LOGIN_STATUS_CONNECTERROR	2
#define LOGIN_STATUS_SOCKERROR		3
#define M_VIDEOMSG_PREDEFINED_SIZE 5
#define M_SCROLLMSG_PREDEFINED_SIZE 5

STcpServer *Sockserver = NULL;
//消息下发处理
static bool msgclient_onpacket(HANDLE Client, unsigned long ip, unsigned short port, char *buf, int size, unsigned long lParam )
{
	//STcpClient *m_Client = (STcpClient *)Client;
	printf("size:%d recv:%s\n", size, buf);
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
