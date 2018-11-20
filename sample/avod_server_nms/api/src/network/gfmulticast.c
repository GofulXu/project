/** 
 * @file   gfmulticast.c
 * @brief 
 * @author 
 * @date 
 * @version 1.0.0.0
 */

#include "gfapi.h"
#include "gfthrd.h"
#include "gfparamerter.h"
#include "gflog.h"
#include "gfreportlog.h"
#include "gfmulticast.h"

static bool m_exit = false;

static int socket_init(NetworkClient_t *pNetworkClient, bool is_recv)
{
	int sockfd = -1;
	struct sockaddr_in local_addr;
	unsigned int socklen = sizeof(local_addr);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd <= 0)
	{
		GFMULTICAST_LOG_DEBUG("socket creating error\n");
		return -1;
	}
	if(UNICAST == pNetworkClient->network_type)
	{
		printf("unicast!\n");
		return sockfd;
	}

	memset(&local_addr, 0, socklen);
	local_addr.sin_family = AF_INET;

	if(BROADCAST == pNetworkClient->network_type)
		local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		local_addr.sin_addr.s_addr = pNetworkClient->dest_ip;

	if(is_recv)
		local_addr.sin_port = pNetworkClient->dest_port; 		//接收时绑定目的端口
	else
		local_addr.sin_port = pNetworkClient->src_port; 		//发送时绑定源端口

	if (bind(sockfd, (struct sockaddr *) &local_addr, socklen) < 0) 
	{
		GFMULTICAST_LOG_ERROR("Bind error; error num %s\n", strerror(errno));
        close(sockfd);
        sockfd = -1;
		return -1;
	}
	if(BROADCAST == pNetworkClient->network_type)
	{
		//set broadcast param
		int broadcast_en = 1;
		if(setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,&broadcast_en,sizeof(broadcast_en)) != 0)
		{
			gf_reportlog_save(BROADCAST_MODULE, "setsockopt broadcast property error\n");
			return -1;
		}
	}
	else
	{
		//set multicast param
		struct ip_mreq mreq;
		mreq.imr_multiaddr.s_addr = pNetworkClient->dest_ip;
		mreq.imr_interface.s_addr = pNetworkClient->local_ip;
		if(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)))
		{
			GFMULTICAST_LOG_ERROR("IP_ADD_MEMBERSHIP failed;error num %d\n", errno);
			close(sockfd);
			sockfd = -1;
			return -1;
		}
		
		int flag = 1;
		if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&flag, sizeof(flag)))
		{
			GFMULTICAST_LOG_ERROR("add IP_MULTICAST_LOOP failed;error num\n");
			return -1;
		}
		
		int ttl = 8;
		if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl)))
		{
			GFMULTICAST_LOG_ERROR("add IP_MULTICAST_TTL failed;error num \n");
			return -1;
		}
	}

    return sockfd;
}

static bool recv_data(uint64_t wParam, uint64_t lParam)
{
	int ret = 0;
	int last_recv_time = gf_thrd_get_tick();
	fd_set readFds;
	struct timeval tv;
	struct sockaddr_in from;
	NetworkClient_t *pNetworkClient = (NetworkClient_t *)wParam;

	while(1)
	{
		if( pNetworkClient->exit )
        {
            if(pNetworkClient->recv_socket > 0)
                close(pNetworkClient->recv_socket);
            pNetworkClient->recv_socket = -1;
            return false;
        }

		if(gf_thrd_get_tick()- last_recv_time > pNetworkClient->timeout)
        {
			if(pNetworkClient->pTimeoutHandler)
				pNetworkClient->pTimeoutHandler(0);
            last_recv_time = gf_thrd_get_tick();
        }
		socklen_t slen = sizeof( from );
		memset(pNetworkClient->recv_buf, 0, sizeof(pNetworkClient->recv_buf));
		memset( &from, 0, sizeof(from) );

		FD_ZERO( &readFds );
		FD_SET(pNetworkClient->recv_socket, &readFds );

		tv.tv_sec = 2;
		tv.tv_usec = 0;
		int activeCount = select(pNetworkClient->recv_socket+1,&readFds ,NULL, NULL, &tv);
		if( activeCount == -1 )
		{
			GFMULTICAST_LOG_DEBUG("station select failed\n\n");
            gf_thrd_delay(200);
			break;
		}
		if( activeCount == 0 )
		{
			//GFMULTICAST_LOG_DEBUG("station select timout out\n");
            gf_thrd_delay(200);
			continue;
		}

		memset(pNetworkClient->recv_buf, 0, sizeof(pNetworkClient->recv_buf));
		ret = recvfrom(pNetworkClient->recv_socket, pNetworkClient->recv_buf, sizeof(pNetworkClient->recv_buf), 0, (struct sockaddr *)&from, &slen );
		if(( ret == 0 )||( ret <= -1 ))
		{
			GFMULTICAST_LOG_DEBUG("recvfrom failed %d %d\n", pNetworkClient->recv_socket, ret);
            gf_thrd_delay(1000);
			continue;
		}
		else
		{
			if(pNetworkClient->pDataHandler)
				pNetworkClient->pDataHandler(pNetworkClient->recv_buf, ret);
			if(pNetworkClient->pAddrHandler)
				pNetworkClient->pAddrHandler(inet_ntoa(from.sin_addr), htons(from.sin_port));
        	last_recv_time = gf_thrd_get_tick();
		}
	}
    gf_thrd_delay(200);
    return true;
}

int gf_multicast_send_data(NetworkClient_t *pNetworkClient, char *data, int size)
{
	struct sockaddr_in serAddr;
	size_t serAddrLen = sizeof(serAddr);

	bzero(&serAddr,serAddrLen);
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = pNetworkClient->dest_port;
	serAddr.sin_addr.s_addr = pNetworkClient->dest_ip;

	if(pNetworkClient->send_socket <= 0)
		return -1;

	int ret = sendto(pNetworkClient->send_socket, data, size, 0,(struct sockaddr*)&serAddr,sizeof(serAddr));
	if(ret != size)
	{
        gf_reportlog_save(BROADCAST_MODULE, "send data to %s:%d error, ret = %d\n", pNetworkClient->dest_ip,pNetworkClient->dest_port, ret);
		return -1;
	}
	return 0;
}

/**
 * @brief	模块初始化
 * @return  0成功,否则错误码
 */
int gf_multicast_init(NetworkClient_t *pNetworkClient)
{
	pNetworkClient->recv_socket = socket_init(pNetworkClient, true);
	if(pNetworkClient->recv_socket <= 0)
	{
		GFMULTICAST_LOG_DEBUG("recv socket create failed\n");
		return -1;	
	}
	//假如源端口和目的端口使用同一个时，使用同一个socket发送和接收
	if(pNetworkClient->src_port != pNetworkClient->dest_port)
	{
		pNetworkClient->send_socket = socket_init(pNetworkClient, false);
		if(pNetworkClient->send_socket <= 0)
		{
			GFMULTICAST_LOG_DEBUG("send socket create failed\n");
			return -1;	
		}
	}
	else
		pNetworkClient->send_socket = pNetworkClient->recv_socket;
    pNetworkClient->tRecvThrd = gf_thrd_open("NetworkRecvThrd",100,0,4096,recv_data, NULL, (uint64_t)pNetworkClient,0);
	if(pNetworkClient->tRecvThrd)
        gf_thrd_resume(pNetworkClient->tRecvThrd);
    else
        return false;

    return 0;
}

/** 
 * @brief	模块退出 
 * @return  无
 */
int gf_multicast_exit(NetworkClient_t *pNetworkClient)
{
	pNetworkClient->exit = true;

    if(pNetworkClient->recv_socket > 0)
    {
        close(pNetworkClient->recv_socket);
        pNetworkClient->recv_socket = -1;
    }
	if(pNetworkClient->send_socket > 0)
    {
        close(pNetworkClient->send_socket);
        pNetworkClient->send_socket = -1;
    }
    if(pNetworkClient->tRecvThrd)	
    {
        gf_thrd_close(pNetworkClient->tRecvThrd, 100);
        pNetworkClient->tRecvThrd = NULL;
    }
	return 0;
}


