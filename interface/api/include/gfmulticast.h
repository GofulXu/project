#ifndef GFMULTICAST_H
#define GFMULTICAST_H

#ifdef __cplusplus
extern "C"
{
#endif

#define GFMULTICAST_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "gfmulticast", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFMULTICAST_LOG_INFO( format, ...) 	gf_log(LOG_LEVEL_INFO, "gfmulticast", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFMULTICAST_LOG_WARN( format, ...) 	gf_log(LOG_LEVEL_WARN, "gfmulticast", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFMULTICAST_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "gfmulticast", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFMULTICAST_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "gfmulticast", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

typedef int (*PDataHandler)(char *data, int size, unsigned long ip, unsigned short port);

typedef int (*PTimeoutHandler)(int action);

typedef int (*PAddrHandler)(char *addr_ip, int port);

typedef enum _NETWORK_TYPE{
	BROADCAST,
	MULTICAST,
	UNICAST
}NETWORK_TYPE;

typedef struct _networkClient{
	bool exit;
	unsigned long local_ip;
	unsigned long dest_ip;	
	unsigned short dest_port;
	unsigned short src_port;	
	char  src_addr[128];	

	NETWORK_TYPE network_type;
	SOCKET recv_socket;
	char recv_buf[64*1024];
	char recv_size;
	HANDLE tRecvThrd;
	PDataHandler pDataHandler;
	PTimeoutHandler pTimeoutHandler;
	int timeout;
#if defined(SUPPORT_CHANGZHOUYG1) || defined(SUPPORT_CHANGZHOUYG1_DG)
	PTimeoutHandler pTimeoutHandler2;
	int timeout2;
#endif
	PAddrHandler pAddrHandler;


	SOCKET send_socket;
	char send_buf[64*1024];
	int send_size;
	HANDLE tSendThrd;
}NetworkClient_t;

/**
*@breif     gfmulticast init
*/
int gf_multicast_init(NetworkClient_t *pNetworkClient);

/**
*@breif     gfmulticast exit
*/
int gf_multicast_exit(NetworkClient_t *pNetworkClient);

int gf_multicast_send_data(NetworkClient_t *pNetworkClient, char *data, int size);

#ifdef __cplusplus
}
#endif

#endif
