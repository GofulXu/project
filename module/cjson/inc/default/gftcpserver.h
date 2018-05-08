#ifndef __GFTCPSERVER2_H__
#define __GFTCPSERVER2_H__

#ifdef __cplusplus
extern "C"{
#endif

#define MAX_CLIENT_NUM 200

typedef bool (*PPacketHandler)( HANDLE client, unsigned long ip,
			unsigned short port, char *buf, int size, unsigned long lParam );

typedef bool (*PErrorHandler)( HANDLE client, unsigned long ip,
			unsigned short port, unsigned long lParam );

typedef struct
{
    long unsigned int ip;
    unsigned short port;
	SOCKET socket;
	char buf[1024];
	char sndbuf[8*1024];
    bool is_use;
	HANDLE hThrd;
	HANDLE mutex;
	/*侦听线程*/
	PPacketHandler pPacketHandler;
	PErrorHandler pErrorHandler;	
}STcpClient;

typedef struct
{
	/*接收地址*/
	unsigned long ip;
	/*接收端口*/
	unsigned short port;
	/*工作套解字*/
	SOCKET socket;
    /*服务器收到的客户端数*/
    int m_client_num;
    STcpClient m_client[MAX_CLIENT_NUM];
    /*客户端是否使用*/

	unsigned long lParam;
	HANDLE tcpserver_thrd;
	PPacketHandler pPacketHandler;
	PErrorHandler pErrorHandler;	
}STcpServer;
/*************************************************************************
*@param pClient
*@param ip
*@param port
*@param pPacketHandler
*@param lParam
*@return true or flase
*************************************************************************/
STcpServer* gf_tcpserver_init(unsigned long ip, unsigned short port,
		PPacketHandler pPacketHandler, PErrorHandler pErrorHandler,unsigned long lParam );
		
/*************************************************************************
*@param pClient
*@return no
*************************************************************************/
void gf_tcpserver_exit( STcpServer *m_tcpserver );

/*************************************************************************
*@param pClient
*@param buf
*@param size
*@param timeout
*************************************************************************/
int gf_tcpserver_send(STcpClient* pClient, char* buf, int size, int timeout);

void gf_tcpserverheart_send(STcpServer *m_tcpserver, char* buf, int size, int timeout);

bool gf_tcpserver_isopen( STcpServer* pServer );
bool gf_tcpserver_clientisopen( STcpClient* pClient );
bool gf_tcpserver_freeclient( STcpClient* pClient );
#ifdef __cplusplus
}
#endif
	
#endif
