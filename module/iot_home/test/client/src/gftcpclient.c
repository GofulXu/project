#include "gfapi_linux.h"
#include "gflog.h"
#include "gfthrd.h"
#include "gftcp.h"
#include "gftype.h"
#include "gfdefine.h"
#include "gftcpclient.h"

/*数据流接收线程*/
static bool tcpclient_onrecv( uint32_t wparam, uint32_t lparam )
{
	STcpClient*	pClient = (STcpClient*)wparam;
	int rcv_size = 0;
	int buf_size = sizeof(pClient->buf);
	char *p = NULL;
	fd_set readfds;
	pClient->datasize = 0;

	/*清一下接收缓存*/
	/*判断当前网络连接的结构是否有效*/
	if( pClient==NULL )
	{
		HWTCPCLIENT_LOG_DEBUG( "[%s:%d]Invalid Tcp client!\n",__FUNCTION__,__LINE__);
		return false;
	}
#if 0
again:
#endif
	if(gf_tcp_select(pClient->socket, &readfds, NULL, NULL, 2000))
	{

		memset(pClient->buf, 0, sizeof(pClient->buf));
		p = pClient->buf + pClient->datasize;

		/*接收数据*/
		rcv_size = recv( pClient->socket, p, buf_size - pClient->datasize, 0 );

		if(rcv_size > 0)
			pClient->datasize += rcv_size;

		//HWTCPCLIENT_LOG_INFO("\n[%s][%d]%s from [%lu.%lu.%lu.%lu]\n",__FUNCTION__,__LINE__, p,pClient->ip & 0x000000FF,(pClient->ip >> 8) & 0x000000FF,(pClient->ip >> 16) & 0x000000FF,(pClient->ip >> 24) & 0x000000FF);

		/*如果接收的是xml，检查下xml是否完整，如果不完整，继续下次接收*/
#if 0
		if(true == pClient->check_entire)
		{
			if(strstr(pClient->buf, "<?xml"))
			{
			   if(strstr(pClient->buf, "/command>")) 
			   {
					if(pClient->pPacketHandler)
					{	
						if(!pClient->pPacketHandler(pClient->socket, pClient->ip, pClient->port, pClient->buf, pClient->datasize, pClient->lParam) )
						{
							if(pClient->databuf)
							{
								free(pClient->databuf);
								pClient->databuf = NULL;
							}
						}
					}
			   }
			   else
			   {
					// no finish recv
					HWTCPCLIENT_LOG_DEBUG("no recv all datas...,recv again\n");
					goto again;
			   }
			}
			else
			{
				memset(pClient->buf, 0, sizeof(pClient->buf));
				pClient->datasize = 0;
			}
		}
		else //不检查时直接送数据给处理函数
#endif
		if(rcv_size > 0)
		{
			if(pClient->pPacketHandler)
			{	
				if(!pClient->pPacketHandler(pClient->socket, pClient->ip, pClient->port, p, pClient->datasize, pClient->lParam) )
				{
					if( pClient->socket != -1 )
					{
						close( pClient->socket );
						pClient->socket = -1;
					}	
#if 0
					if( pClient->databuf!=NULL)
					{
						free(pClient->databuf);
						pClient->databuf = NULL;
					}
#endif
					pClient->pPacketHandler = NULL;
					pClient->lParam = 0;
					pClient->ip = 0;
					pClient->port = 0;
					printf("[%s:%d]net recive failen\n", __FUNCTION__, __LINE__);
					return false;
				}
			}
		}

		if(rcv_size <= 0)
		{
			if( pClient->socket != -1 )
			{
				close( pClient->socket );
				pClient->socket = -1;
			}	
#if 0
			if( pClient->databuf!=NULL)
			{
				free(pClient->databuf);
				pClient->databuf = NULL;
			}
#endif
			pClient->pPacketHandler = NULL;
			pClient->lParam = 0;
			pClient->ip = 0;
			pClient->port = 0;
			printf("[%s:%d]net recive failen\n", __FUNCTION__, __LINE__);
			return false;
		}
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
bool gf_tcpclient_init( STcpClient* pClient,unsigned long ip,unsigned short port,PPacketHandler pPacketHandler,threadhandler_t pErrorHandler,uint32_t lParam )
{
	struct sockaddr_in dest;
	SOCKET skt=-1;
	fd_set  rset, wset;
	unsigned int now = 0;
	struct linger lingerOpt;
	int i = 0,n = 0,retcode = 0,unblock = 1,optval = 32*1024,reuse = 1;

	if( !pClient ) return false;
	memset( pClient, 0, sizeof(STcpClient) );
	pClient->socket = -1;
	pClient->hThrd = NULL;
	pClient->ip = ip;
	pClient->port = port;
	pClient->pPacketHandler = pPacketHandler;
	pClient->lParam = lParam;
	pClient->check_entire = false;
	
	memset( &dest, 0, sizeof(dest) );
	dest.sin_addr.s_addr = ip;
	dest.sin_family = AF_INET;
	dest.sin_port = port;

	for( i=0; i<3;  i++ )
	{
		skt = gf_tcp_socket();
		if( skt == -1)
			continue;
		/*set receive buf size*/
		setsockopt(skt,SOL_SOCKET,SO_RCVBUF,(void*)&optval,sizeof(optval));
		/* 设为非阻塞模式 */
		if(gf_tcp_ioctl( skt, FIONBIO, (uint32_t*)&unblock ) < 0)
		{
			closesocket(skt);
            skt = -1;
			continue;
		}
		lingerOpt.l_onoff = 1;
		lingerOpt.l_linger = 0;
		/*set can reuse loacl address*/
		setsockopt(skt , SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
		/*if there are data be sent ,delay to shutdown the connection*/
		setsockopt( skt, SOL_SOCKET, SO_LINGER,(void*) &lingerOpt, sizeof(lingerOpt) );

		//if( first )
		{
			now = gf_thrd_get_tick();
			srand( now );
			/* get a rand port*/
			gf_tcp_bind(skt, INADDR_ANY, htons( (unsigned short) (30000 + rand()%26000 )));
		}
		/* 连接... */
		errno = 0;
		retcode = gf_tcp_connect( skt, ip, port );
		// if connection is ok ,break the loop;
		if(retcode == 0)
			break;
		if(errno != 0 && errno != EINPROGRESS)
		{
			closesocket(skt);
			HWTCPCLIENT_LOG_ERROR("[%s]connect retcode:%d errorcode=%d,skt = %d\n", __FUNCTION__,retcode, errno,skt);
            skt = -1;
			continue;
		}
		n=0;
WAIT:
		/* 等待连接成功 */
		if( (retcode = gf_tcp_select(skt, &rset, &wset, NULL, 6000 )) < 0 )
		{
			closesocket(skt);
            HWTCPCLIENT_LOG_ERROR("[%s]tcp select error retcode:%d,skt = %d\n",__FUNCTION__,retcode,skt);
            skt = -1;
			continue ;
		}

		if(retcode == 0)
		{
			HWTCPCLIENT_LOG_ERROR("[%s:%d]select socket timeout!\n", __FUNCTION__, __LINE__);
			closesocket(skt);
            skt = -1;
			continue;
		}

		if( FD_ISSET(skt, &rset ) )
		{
			char szBuf[16];
			int readsize = gf_tcp_recv( skt, szBuf, sizeof(szBuf) );
			if( readsize==-1 && (errno==EWOULDBLOCK || errno==EINPROGRESS) )
			{
				if( ++n<3 )
					goto WAIT;
			}
		}
		else if( FD_ISSET( skt, &wset ) )
		{
			int err = 0;
			int len = sizeof(err);
#ifndef WIN32
			if(getsockopt(skt, SOL_SOCKET, SO_ERROR, &err, (socklen_t *)&len) < 0)
#else
				if(getsockopt(skt, SOL_SOCKET, SO_ERROR, (char*)&err, &len) < 0)
#endif
				{
					closesocket( skt );
					HWTCPCLIENT_LOG_ERROR("[%s:%d]getsocketopt failed!skt:%d\n",__FUNCTION__, __LINE__,skt);
                    skt = -1;
					continue;
				}

			if(err) 
			{
				closesocket( skt );
				HWTCPCLIENT_LOG_ERROR("[%s:%d] connect failed erron:%d,skt:%d!\n", __FUNCTION__, __LINE__, err,skt);
                skt = -1;
				continue;
			}
			break;
		}
		closesocket( skt );
		skt = -1;
	}
	if(skt<0)
	{
		HWTCPCLIENT_LOG_DEBUG("[%s:%d]cannot connect server!\n", __FUNCTION__, __LINE__ );
		gf_tcpclient_exit(pClient);
		return false;
	}
	pClient->socket = skt;
	/*设回阻塞方式*/
	unblock = 0;
	ioctl( pClient->socket, FIONBIO, (int*)&unblock );

	pClient->hThrd = gf_thrd_open( "TcpClientRecv", 50, 0, 256*1024,/*(PThreadHandler)*/tcpclient_onrecv, pErrorHandler, (uint32_t)pClient, lParam);
	if( pClient->hThrd == NULL )
	{
		HWTCPCLIENT_LOG_DEBUG( "[%s:%d]cannot create task !\n", __FUNCTION__, __LINE__ );
		gf_tcpclient_exit(pClient);
		return false;
	}
	gf_thrd_resume( pClient->hThrd );
	HWTCPCLIENT_LOG_DEBUG("[%s:%d]connect server(%x:%x) suc.\n", __FUNCTION__, __LINE__, ip, port);
	return true;
}

/*关闭接收*/
void gf_tcpclient_exit( STcpClient *pClient )
{
	if( pClient->socket != -1 )
	{
		close( pClient->socket );
		pClient->socket = -1;
	}	
	/*强行中止接收线程*/
	if( pClient->hThrd != NULL )
	{
		printf("test %s:%d \n", __FUNCTION__, __LINE__);
		gf_thrd_close( pClient->hThrd, 1000);
		pClient->hThrd = NULL;
	}
#if 0
	if( pClient->databuf!=NULL)
	{
		free(pClient->databuf);
		pClient->databuf = NULL;
	}
#endif
	pClient->pPacketHandler = NULL;
	pClient->lParam = 0;
	pClient->ip = 0;
	pClient->port = 0;
}

/*数据发送*/
int gf_tcpclient_send(STcpClient* pClient, char* buf, int size, int timeout)
{
	int ret = -1;
	
    HWTCPCLIENT_LOG_INFO("tcpclient = 0x%x,socket = %d\n",pClient,pClient->socket);	
	if( pClient && pClient->socket != INVALID_SOCKET )
	{
		/*设置发送超时*/
//		if(setsockopt( pClient->socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout) ) == 0)
		/*发送负载*/
			ret = send( pClient->socket, buf, size, 0 );
	}
    HWTCPCLIENT_LOG_INFO("[%s] ret = %d\n",__FUNCTION__,ret);
	return ret;
}


/*是否已经开始接收*/
bool gf_tcpclient_isinit( STcpClient* pClient )
{
	return pClient->socket == -1 ? false : true;
}




