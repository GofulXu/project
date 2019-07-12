#ifndef __GFTCPCLIENT_H__
#define __GFTCPCLIENT_H__

#ifdef __cplusplus
extern "C"{
#endif

#define HWTCPCLIENT_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "gftcpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define HWTCPCLIENT_LOG_INFO( format, ...) 	gf_log(LOG_LEVEL_INFO, "gftcpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define HWTCPCLIENT_LOG_WARN( format, ...) 	gf_log(LOG_LEVEL_WARN, "gftcpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define HWTCPCLIENT_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "gftcpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define HWTCPCLIENT_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "gftcpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)


typedef bool (*PPacketHandler)( SOCKET socket, unsigned long ip,
			unsigned short port, char *buf, int size, unsigned long lParam );

typedef bool (*PErrorHandler)( SOCKET socket, unsigned long ip,
			unsigned short port, unsigned long lParam );

typedef struct
{
	/*接收地址*/
	unsigned long ip;
	/*接收端口*/
	unsigned short port;
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
}STcpClient;
/*************************************************************************
*@param pClient
*@param ip
*@param port
*@param pPacketHandler
*@param lParam
*@return true or flase
*************************************************************************/
bool gf_tcpclient_init( STcpClient* pClient, unsigned long ip, unsigned short port,
		PPacketHandler pPacketHandler, PErrorHandler pErrorHandler,unsigned long lParam );
		
/*************************************************************************
*@param pClient
*@return no
*************************************************************************/
void gf_tcpclient_exit( STcpClient* pClient );

/*************************************************************************
*
*@param pClient
*
*************************************************************************/
bool gf_tcpclient_isinit( STcpClient* pClient );

/*************************************************************************
*@param pClient
*@param buf
*@param size
*@param timeout
*************************************************************************/
int gf_tcpclient_send(STcpClient* pClient, char* buf, int size, int timeout);


#ifdef __cplusplus
}
#endif
	
#endif
