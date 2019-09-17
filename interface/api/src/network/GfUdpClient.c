#include "gfapi.h"
#include "gfthrd.h"
#include "gfudp.h"
#include "GfUdpClient.h"
#include "gflog.h"

/*数据流接收线程*/
static bool UdpClientRecv( uint64_t wparam, uint64_t lparam )
{
	SUdpClient* pClient = (SUdpClient*)wparam;
	fd_set readfds;

	/*清一下接收缓存*/
	/*判断当前网络连接的结构是否有效*/
	if( pClient==NULL )
	{
		GFUDPCLIENT_LOG_DEBUG( "[%s:%d]Invalid udp client!\n",__FUNCTION__,__LINE__);
		return false;
	}
	if(0 < gf_udp_select(pClient->NetP.Socket, &readfds, NULL, NULL, 1000))
	{

		memset(pClient->NetP.Buf, 0, sizeof(pClient->NetP.Buf));
		/*接收数据*/
		pClient->NetP.Size = gf_udp_recv( pClient->NetP.Socket, &pClient->NetP.Ip, &pClient->NetP.Port, pClient->NetP.Buf, sizeof(pClient->NetP.Buf));

		if(pClient->NetP.Size > 0)
		{
			if(pClient->pPacketHandler)
			{	
				if(0 > pClient->pPacketHandler(&pClient->NetP, pClient->lParam) )
				{
					pClient->NetP.Size = 0;
					pClient->hThrd = NULL;
					closesocket(pClient->NetP.Socket);
					pClient->NetP.Socket = INVALID_SOCKET;
					GFUDPCLIENT_LOG_DEBUG( "return false %s:%d\n", __FUNCTION__, __LINE__);
					return false;
				}
			}
		}

		if(pClient->NetP.Size <= 0)
		{
			GFUDPCLIENT_LOG_DEBUG( "[%s:%d]net recive failed\n", __FUNCTION__, __LINE__);
			pClient->NetP.Size = 0;
			pClient->hThrd = NULL;
			if( pClient->pErrorHandler )
			{
				pClient->pErrorHandler(&pClient->NetP, pClient->lParam);
				closesocket(pClient->NetP.Socket);
				pClient->NetP.Socket = INVALID_SOCKET;
				/*退出线程*/
				GFUDPCLIENT_LOG_DEBUG( "return false %s:%d\n", __FUNCTION__, __LINE__);
			}else
			{
				closesocket(pClient->NetP.Socket);
				pClient->NetP.Socket = INVALID_SOCKET;
				/*退出线程*/
				GFUDPCLIENT_LOG_DEBUG( "return false %s:%d\n", __FUNCTION__, __LINE__);
			}
			return false;
		}
	}

	return true;
}

/*关闭接收*/
void GfUdpClientExit( SUdpClient *pClient )
{
	if( pClient->NetP.Socket != INVALID_SOCKET )
	{
		close( pClient->NetP.Socket );
		pClient->NetP.Socket = INVALID_SOCKET;
	}	
	/*强行中止接收线程*/
	if( pClient->hThrd != NULL )
	{
		gf_thrd_close( pClient->hThrd, 2000 );
		pClient->hThrd = NULL;
	}
	pClient->pPacketHandler = NULL;
	pClient->pErrorHandler = NULL;
	pClient->lParam = 0;
	pClient->LocalIp = 0;
	pClient->LocalPort = 0;
	pClient->HostIp = 0;
	pClient->HostPort = 0;
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
	SOCKET Skt=-1;
	//unsigned int now = 0;

	if( !pClient ) return false;
	pClient->hThrd = NULL;
	pClient->Mutex = NULL;
	pClient->NetP.Socket = INVALID_SOCKET;
	
	Skt = gf_udp_socket();
	if( Skt == INVALID_SOCKET)
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
	if(0 > gf_udp_bind(Skt, pClient->LocalIp, pClient->LocalPort))
	{
		GFUDPCLIENT_LOG_DEBUG( "%s: bind %d err\n", __FUNCTION__, ntohs(pClient->LocalPort));
		GfUdpClientExit(pClient);
		return -2;
	}
	GFUDPCLIENT_LOG_DEBUG( "%s: bind %d suc\n", __FUNCTION__, ntohs(pClient->LocalPort));
	pClient->NetP.Socket = Skt;
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
int GfUdpClientSend(SUdpClient* pClient, unsigned long HostIp, unsigned short HostPort, char* buf, int size, int timeout)
{
	fd_set writefds;
	if( pClient && pClient->NetP.Socket != INVALID_SOCKET )
	{
		if(0 < gf_udp_select(pClient->NetP.Socket, NULL, &writefds, NULL, timeout))
		{
		    struct sockaddr_in from;
		    memset( &from, 0, sizeof(from) );
		    from.sin_addr.s_addr = HostIp;
		    from.sin_port = HostPort;
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
		    return gf_udp_send( pClient->NetP.Socket, HostIp, HostPort, buf, size);
		}
	}
	return -2;
}


/*是否已经开始接收*/
int GfUdpClientIsInit( SUdpClient* pClient )
{
	if(!pClient || pClient->NetP.Socket == INVALID_SOCKET || !pClient->hThrd)
		return -1;
	return 0;
}
