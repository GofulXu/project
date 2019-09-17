#ifndef __GFUDPCLIENT_H__
#define __GFUDPCLIENT_H__

#ifdef __cplusplus
extern "C"{
#endif

#define GFUDPCLIENT_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "gfudpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFUDPCLIENT_LOG_INFO( format, ...) 	gf_log(LOG_LEVEL_INFO, "gfudpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFUDPCLIENT_LOG_WARN( format, ...) 	gf_log(LOG_LEVEL_WARN, "gfudpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFUDPCLIENT_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "gfudpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFUDPCLIENT_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "gfudpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

typedef struct _NetPacket{
    SOCKET Socket;
    unsigned long Ip;
    unsigned short Port;
    char Buf[128*1024];
    unsigned Size;
}NetPacket;


typedef int (*PPacketHandler)( NetPacket *NetP, unsigned long lParam );

typedef int (*PErrorHandler)( NetPacket *NetP, unsigned long lParam );

typedef struct
{
	/*本地地址*/
	unsigned long LocalIp;
	/*本地端口*/
	unsigned short LocalPort;
	/*主机地址*/
	unsigned long HostIp;
	/*主机端口*/
	unsigned short HostPort;

	NetPacket NetP;
	/*侦听线程*/
	HANDLE hThrd;
	HANDLE Mutex;
	PPacketHandler pPacketHandler;
	PErrorHandler pErrorHandler;	
	unsigned long lParam;
}SUdpClient;
/*************************************************************************
*@param pClient
*@param localip
*@param localport
*@param hostip
*@param hostport
*@param pPacketHandler
*@param lParam
*@return true or flase
*************************************************************************/
int GfUdpClientInit( SUdpClient* pClient);
		
/*************************************************************************
*@param pClient
*@return no
*************************************************************************/
void GfUdpClientExit( SUdpClient* pClient );

/*************************************************************************
*
*@param pClient
*
*************************************************************************/
int GfUdpClientIsInit( SUdpClient* pClient );

/*************************************************************************
*@param pClient
*@param hostip
*@param hostport
*@param buf
*@param size
*@param timeout
*************************************************************************/
int GfUdpClientSend(SUdpClient* pClient, unsigned long hostip, unsigned short hostport, char* buf, int size, int timeout);


#ifdef __cplusplus
}
#endif
	
#endif
