#include "gfapi.h"
#include "gfthrd.h"
#include "gfudp.h"
#include "GfUdpClient.h"
#include "gflog.h"

/*数据流接收线程*/
static bool UdpClientRecv( uint64_t wparam, uint64_t lparam )
{
	SUdpClient*	pClient = (SUdpClient*)wparam;
	int rcv_size = 0;
	int buf_size = sizeof(pClient->buf);
	char *p = NULL;
	unsigned long ip;
	unsigned short port;
	fd_set readfds;
	pClient->datasize = 0;

	/*清一下接收缓存*/
	/*判断当前网络连接的结构是否有效*/
	if( pClient==NULL )
	{
		GFUDPCLIENT_LOG_DEBUG( "[%s:%d]Invalid udp client!\n",__FUNCTION__,__LINE__);
		return false;
	}
	if(0 < gf_udp_select(pClient->socket, &readfds, NULL, NULL, 1000))
	{

		memset(pClient->buf, 0, sizeof(pClient->buf));
		p = pClient->buf + pClient->datasize;

		/*接收数据*/
		rcv_size = gf_udp_recv( pClient->socket, &ip, &port, p, buf_size - pClient->datasize);

		if(rcv_size > 0)
		{
			pClient->datasize += rcv_size;
			if(pClient->pPacketHandler)
			{	
				if(0 > pClient->pPacketHandler(pClient->socket, ip, port, p, pClient->datasize, pClient->lParam) )
				{
					closesocket(pClient->socket);
					pClient->socket = INVALID_SOCKET;
					if(pClient->databuf)
					{
						free(pClient->databuf);
						pClient->databuf = NULL;
					}
					GFUDPCLIENT_LOG_DEBUG( "return false %s:%d\n", __FUNCTION__, __LINE__);
					return false;
				}
			}
		}

		if(rcv_size <= 0)
		{
			GFUDPCLIENT_LOG_DEBUG( "[%s:%d]net recive failed\n", __FUNCTION__, __LINE__);
			pClient->datasize = 0;
			pClient->hThrd = NULL;
			if( pClient->pErrorHandler )
			{
				pClient->pErrorHandler(pClient->socket, pClient->localip, pClient->localport, pClient->lParam);
				closesocket(pClient->socket);
				pClient->socket = INVALID_SOCKET;
				if(pClient->databuf)
				{
					free(pClient->databuf);
					pClient->databuf = NULL;
				}
				/*退出线程*/
				GFUDPCLIENT_LOG_DEBUG( "return false %s:%d\n", __FUNCTION__, __LINE__);
				return false;
			}else
			{
				closesocket(pClient->socket);
				pClient->socket = INVALID_SOCKET;
				if(pClient->databuf)
				{
					free(pClient->databuf);
					pClient->databuf = NULL;
				}
				/*退出线程*/
				GFUDPCLIENT_LOG_DEBUG( "return false %s:%d\n", __FUNCTION__, __LINE__);
				return false;
			}
		}
	}

	return true;
}

/*关闭接收*/
void GfUdpClientExit( SUdpClient *pClient )
{
	if( pClient->socket != INVALID_SOCKET )
	{
		close( pClient->socket );
		pClient->socket = INVALID_SOCKET;
	}	
	/*强行中止接收线程*/
	if( pClient->hThrd != NULL )
	{
		gf_thrd_close( pClient->hThrd, 2000 );
		pClient->hThrd = NULL;
	}
	if( pClient->databuf != NULL)
	{
		free(pClient->databuf);
		pClient->databuf = NULL;
	}
	pClient->pPacketHandler = NULL;
	pClient->pErrorHandler = NULL;
	pClient->lParam = 0;
	pClient->localip = 0;
	pClient->localport = 0;
	pClient->hostip = 0;
	pClient->hostport = 0;
}

/*************************************************************************
 * FUNCTION: Initialize()
 * PURPOSE: 初始化网络
 * PARAM:	addr - 服务器地址，网络字节顺序
			port - 服务器侦听端口，网络字节顺序
 * RETURN: 是否成功
 * NOTES: 
 *************************************************************************/
int GfUdpClientInit( SUdpClient* pClient)
{
	SOCKET skt=-1;
	//unsigned int now = 0;

	if( !pClient ) return false;
	pClient->hThrd = NULL;
	pClient->databuf = NULL;
	pClient->mutex = NULL;
	pClient->socket = INVALID_SOCKET;
	
	skt = gf_udp_socket();
	if( skt == INVALID_SOCKET)
	{
		GFUDPCLIENT_LOG_DEBUG( "%s: create socket err\n", __FUNCTION__);
		return -1;
	}
#if 0
	/*set receive buf size*/
	setsockopt(skt,SOL_SOCKET,SO_RCVBUF,(void*)&optval,sizeof(optval));
	now = gf_thrd_get_tick();
	srand( now );
	gf_udp_bind(skt, INADDR_ANY, htons( (unsigned short) (30000 + rand()%26000 )));
#endif
	if(0 > gf_udp_bind(skt, pClient->localip, pClient->localport))
	{
		GFUDPCLIENT_LOG_DEBUG( "%s: bind %d err\n", __FUNCTION__, ntohs(pClient->localport));
		GfUdpClientExit(pClient);
		return -2;
	}
	GFUDPCLIENT_LOG_DEBUG( "%s: bind %d suc\n", __FUNCTION__, ntohs(pClient->localport));
	pClient->socket = skt;
	pClient->hThrd = gf_thrd_open( "udpClientRecv", 50, 0, 256*1024,/*(PThreadHandler)*/UdpClientRecv, NULL, (uint64_t)pClient, pClient->lParam);
	if( pClient->hThrd == NULL )
	{
		GFUDPCLIENT_LOG_DEBUG( "[%s:%d]cannot create task !\n", __FUNCTION__, __LINE__ );
		GfUdpClientExit(pClient);
		return -3;
	}
	gf_thrd_resume( pClient->hThrd );
	return 0;
}

/*数据发送*/
int GfUdpClientSend(SUdpClient* pClient, unsigned long hostip, unsigned short hostport, char* buf, int size, int timeout)
{
	fd_set writefds;
	if( pClient && pClient->socket != INVALID_SOCKET )
	{
		if(0 < gf_udp_select(pClient->socket, NULL, &writefds, NULL, timeout))
		{
		    struct sockaddr_in from;
		    memset( &from, 0, sizeof(from) );
		    from.sin_addr.s_addr = hostip;
		    from.sin_port = hostport;
#if 0
		    int i = 0;
		    printf("udp_send buf: ");
		    for(i = 0; i < size; i++)
		    {
			printf("%02x ", buf[i]);
		    }
		    printf("\n");
		    printf("%s:%d---ip:%s\tport:%d\n", __FUNCTION__, __LINE__, inet_ntoa(from.sin_addr), htons(from.sin_port));
#endif
		    return gf_udp_send( pClient->socket, hostip, hostport, buf, size);
		}
	}
	return -2;
}


/*是否已经开始接收*/
bool GfUdpClientIsInit( SUdpClient* pClient )
{
	if(!pClient || pClient->socket == INVALID_SOCKET || !pClient->hThrd)
		return false;
	return true;
}
