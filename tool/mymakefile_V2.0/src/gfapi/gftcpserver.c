#include "gfapi_linux.h"
#include "gflog.h"
#include "gfthrd.h"
#include "gfmutex.h"
#include "gftcp.h"
#include "gfdefine.h"
#include "gftcpserver.h"

void tcpclient_exit( STcpClient *pClient)
{ 
	pClient->ip = 0;
	pClient->port = 0;
	if( pClient->socket != -1 )
	{
		close( pClient->socket );
		pClient->socket = -1;
	}	
	/*强行中止接收线程*/
	if( pClient->hThrd != NULL )
	{
		gf_thrd_close( pClient->hThrd, 0);
		pClient->hThrd = NULL;
	}
	pClient->pPacketHandler = NULL;
	pClient->pErrorHandler = NULL;
	//pClient->lParam = 0;
	pClient->is_use = false;
}

/*数据流接收线程*/
static bool tcpclient_onrecv( uint64_t wparam, uint64_t lparam )
{
	STcpClient*	pClient = (STcpClient*)wparam;
	int rcv_size = 0;
	int buf_size = sizeof(pClient->buf);
	char *p = NULL;

	/*清一下接收缓存*/
	/*判断当前网络连接的结构是否有效*/
	if( pClient==NULL || pClient->socket==-1 )
	{
		printf("pclient == null\n");
		return false;
	}
	p = pClient->buf;
	memset(pClient->buf, 0, sizeof(pClient->buf));
	/*接收数据*/
	rcv_size = recv( pClient->socket, p, buf_size, 0 );
	if(pClient->pPacketHandler && rcv_size > 0)
	{	
		if(!pClient->pPacketHandler((HANDLE)pClient, pClient->ip, pClient->port, p, rcv_size, 0) )
		{
			pClient->ip = 0;
			pClient->port = 0;
			if( pClient->socket != -1 )
			{
				close( pClient->socket );
				pClient->socket = -1;
			}	
			pClient->pPacketHandler = NULL;
			pClient->pErrorHandler = NULL;
			//pClient->lParam = 0;
			pClient->is_use = false;
			printf("packet handler return false\n");
			return false;
		}
	}
	if(rcv_size <= 0)
	{
		printf("recv size <= 0 %s:%d\n", __FUNCTION__, __LINE__);
		pClient->ip = 0;
		pClient->port = 0;
		if( pClient->socket != -1 )
		{
			close( pClient->socket );
			pClient->socket = -1;
		}	
		pClient->pPacketHandler = NULL;
		pClient->pErrorHandler = NULL;
		//pClient->lParam = 0;
		pClient->is_use = false;
		printf("packet handler return false\n");
		return false;
	}
	return true;
}
static bool tcpserver_proc(uint64_t lparam, uint64_t wparam)
{

	int i = 0;
	STcpServer *tcpserver = (STcpServer *)lparam;
    STcpClient *pClient = NULL;

    STcpClient *pTempClient = (STcpClient *)malloc(sizeof(STcpClient));

	memset(pTempClient, 0, sizeof(STcpClient));

    pTempClient->socket = gf_tcp_accept(tcpserver->socket,&pTempClient->ip,&pTempClient->port);
	if(0 == pTempClient->port)
	{
		gf_tcp_close( pTempClient->socket );
		pTempClient->socket = -1;
		printf( "find the invalid client ,ip = %lu.%lu.%lu.%lu,port = %d\n", pTempClient->ip & 0xFF,(pTempClient->ip >> 8) & 0xFF,(pTempClient->ip >> 16) & 0xFF,(pTempClient->ip >> 24) & 0xFF,pTempClient->port);
		gf_thrd_delay(200);
		goto ERROR;
	}

    printf( "accept new client,ip = %lu.%lu.%lu.%lu,port = %d allnum:%d\n", pTempClient->ip & 0xFF,(pTempClient->ip >> 8) & 0xFF,(pTempClient->ip >> 16) & 0xFF,(pTempClient->ip >> 24) & 0xFF,pTempClient->port,tcpserver->m_client_num);

    for(i = 0; i < tcpserver->m_client_num; i ++) 
    {
        if( true == tcpserver->m_client[i].is_use && tcpserver->m_client[i].ip == pTempClient->ip && tcpserver->m_client[i].port == pTempClient->port)
        {
			printf("find the same client at postion:%d %ld:%ld %d:%d\n", i, tcpserver->m_client[i].ip, pTempClient->ip, tcpserver->m_client[i].port, pTempClient->port);
			if(tcpserver->m_client[i].socket != pTempClient->socket)
				tcpclient_exit( &tcpserver->m_client[i] );
			else
			{
				printf("same socket\n");
				pTempClient->socket = -1;
				goto ERROR;
			}
			break;
        }
    }

	for(i = 0; i < tcpserver->m_client_num; i ++) 
	{
		if(false == tcpserver->m_client[i].is_use )
		{
			printf("this is a new client at position [%d]\n",i);
			pClient = &tcpserver->m_client[i];
			break;
		}
	}
	//判断客户端数据，如果太多，则等待释放
	if(i >= tcpserver->m_client_num)
	{
		printf("tcp client too many, more than %d\n",tcpserver->m_client_num);
		goto ERROR;
	}

	pClient->socket = pTempClient->socket;
	pClient->ip = pTempClient->ip;
	pClient->port = pTempClient->port;

	if(pClient->mutex == NULL)
    	pClient->mutex = gf_mutex_create();
	pClient->is_use = true;
	pClient->pPacketHandler = tcpserver->pPacketHandler;
	pClient->pErrorHandler = tcpserver->pErrorHandler;
	pClient->hThrd = gf_thrd_open( "TcpClientRecv", DEFAULT_PRIORIOTY, GF_SCHED_NORMAL, DEFAULT_STACK_SIZE, tcpclient_onrecv, NULL, (unsigned long)pClient, 0);
	if(NULL != pClient->hThrd)
	{
		gf_thrd_resume(pClient->hThrd);
	}else
	{
		printf("TcpClientRecv open err\n");
		goto ERROR;
	}


	if(NULL != pTempClient)
	{
		free(pTempClient);
		pTempClient = NULL;
	}
	printf("TcpClientRecv open suc\n");

	gf_thrd_delay(1000);
	return true;
ERROR:
	fflush(stdout);
	if(-1 != pTempClient->socket)
	{
		gf_tcp_close(pTempClient->socket);
		pTempClient->socket = -1;
	
	}
		
	if(NULL != pTempClient)
	{
		free(pTempClient);
		pTempClient = NULL;
	}
	return true;
}

/*************************************************************************
 * FUNCTION: Initialize()
 * PURPOSE: 初始化网络
 * PARAM:	addr - 服务器地址，网络字节顺序
			port - 服务器侦听端口，网络字节顺序
 * RETURN: 是否成功
 * NOTES: 
 *************************************************************************/
STcpServer * gf_tcpserver_init(unsigned long ip, unsigned short port, PPacketHandler pPacketHandler,PErrorHandler pErrorHandler, unsigned long lParam)
{
	STcpServer *m_tcpserver;
    m_tcpserver = (STcpServer *)malloc(sizeof(STcpServer));
    if(m_tcpserver == NULL)
    {
        printf("tcp server malloc error");
        return NULL;
    }
    memset(m_tcpserver, 0, sizeof(STcpServer));

	m_tcpserver->pPacketHandler = pPacketHandler;
	m_tcpserver->pErrorHandler = pErrorHandler;

    m_tcpserver->socket = gf_tcp_socket();
    if( m_tcpserver->socket <= 0)
	{
        printf("tcp server create socket error");
        return NULL;
	}
	if(gf_tcp_bind(m_tcpserver->socket, ip, port) < 0)
    {
        printf("tcp server bind socket error");
        return NULL;
    }

    if(gf_tcp_listen(m_tcpserver->socket,5) < 0)
    {
        printf("tcp server listen error");
        return NULL;
    }
	m_tcpserver->m_client_num  = MAX_CLIENT_NUM;

	m_tcpserver->tcpserver_thrd = gf_thrd_open("tcpserver_thrd",100,0,4096, tcpserver_proc, NULL, (unsigned long)m_tcpserver, 0);

	if(m_tcpserver->tcpserver_thrd)
		gf_thrd_resume(m_tcpserver->tcpserver_thrd);
	else
		return NULL;

	return m_tcpserver;
}

/*关闭接收*/
void gf_tcpserver_exit( STcpServer *m_tcpserver)
{ 
	int i;
	STcpClient *pClient = NULL;
	gf_tcp_close(m_tcpserver->socket);
	gf_thrd_close(m_tcpserver->tcpserver_thrd, 2000);

	for(i = 0; i < m_tcpserver->m_client_num; i ++) 
	{
		if( m_tcpserver->m_client[i].is_use )
		{
			pClient = &m_tcpserver->m_client[i];
			if( pClient->socket != -1 )
			{
				close( pClient->socket );
				pClient->socket = -1;
			}	
			if( pClient->hThrd != NULL )
			{
				gf_thrd_close( pClient->hThrd, 2000 );
				pClient->hThrd = NULL;
			}
			pClient->pPacketHandler = NULL;
			pClient->pErrorHandler = NULL;
			pClient->is_use = false;
		}
	}
		return;
}

/*数据发送*/
void gf_tcpserverheart_send(STcpServer *m_tcpserver, char* buf, int size, int timeout)
{
	int ret = -1;
	int i;
	fd_set writefds;
	STcpClient *pClient = NULL;
	
	for(i = 0; i < m_tcpserver->m_client_num; i ++) 
	{
		pClient = &m_tcpserver->m_client[i];
		if( pClient->socket > 0 && pClient->is_use == true )
		{
			if(gf_tcp_select(pClient->socket, NULL, &writefds, NULL, timeout))
			{
				ret = gf_tcpserver_send( pClient, buf, size, 0);
				if(ret <= 0)
					tcpclient_exit(pClient);
			}
		}
	}
	return;
}

bool gf_tcpserver_freeclient( STcpClient* pClient )
{
	tcpclient_exit(pClient);
	return true;
}

int gf_tcpserver_send(STcpClient* pClient, char* buf, int size, int timeout)
{
	int ret = -1;
	fd_set writefds;
	
	if(gf_tcp_select(pClient->socket, NULL, &writefds, NULL, timeout))
	{
		if( pClient && pClient->socket != INVALID_SOCKET )
			ret = send( pClient->socket, buf, size, 0 );
	}
	return ret;
}


/*是否已经开始接收*/
bool gf_tcpserver_isopen( STcpServer* pServer )
{
	return pServer->socket != -1 || pServer->tcpserver_thrd != NULL ? true : false;
}

/*是否已经开始接收*/
bool gf_tcpserver_clientisopen( STcpClient* pClient )
{
	return pClient->socket != -1 && pClient->hThrd != NULL ? true : false;
}





