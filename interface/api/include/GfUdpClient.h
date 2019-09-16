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


typedef int (*PPacketHandler)( SOCKET socket, unsigned long ip, unsigned short port, char *buf, int size, unsigned long lParam );

typedef int (*PErrorHandler)( SOCKET socket, unsigned long ip,	unsigned short port, unsigned long lParam );

typedef struct
{
	/*本地地址*/
	unsigned long localip;
	/*本地端口*/
	unsigned short localport;
	/*主机地址*/
	unsigned long hostip;
	/*主机端口*/
	unsigned short hostport;
	/*工作套解字*/
	SOCKET socket;
	/*接收缓冲区*/
	char buf[128*1024];
	/*接收数据大小*/
	int datasize;
	/*一个命令缓冲区*/
	char command_buf[8*1024];
	/*发送缓冲区*/
	char sndbuf[64*1024];
	char* databuf;
	/*侦听线程*/
	HANDLE hThrd;
	HANDLE mutex;
	PPacketHandler pPacketHandler;
	PErrorHandler pErrorHandler;	
	unsigned long lParam;
	bool check_entire;
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
bool GfUdpClientIsInit( SUdpClient* pClient );

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
