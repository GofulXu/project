#ifndef SWCOM_H
#define SWCOM_H

#ifdef __cplusplus
extern "C"
{
#endif

#define GFCOM_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "com", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFCOM_LOG_INFO( format, ...) 	gf_log(LOG_LEVEL_INFO, "com", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFCOM_LOG_WARN( format, ...) 	gf_log(LOG_LEVEL_WARN, "com", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFCOM_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "com", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFCOM_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "com", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

typedef int (*PComDataHandler)(unsigned char *data, int size);

typedef int (*PComTimeoutHandler)(int action);

typedef struct _comClient 
{
	//串口
	int uart_port;
	//波特率
	int rate; 
	//接收超时时间
	int timeout;
	//接收和发送SOCKET
	SOCKET socket;
	//接收临时缓冲
	unsigned char recv_buf[128];
	
	//上次接收时间
	int last_recv_time;
	//是否退出
	bool exit;
	//是否需要重启com接收客户端
	bool reboot;
	//是否是调试模式
	bool debug;
	//接收线程
	HANDLE tRecvThrd;
	//接收数据处理函数
	PComDataHandler pComDataHandler;
	//超时处理
	PComTimeoutHandler pComTimeoutHandler;
}ComClient_t;

//初始化一个COM实例
int gf_com_init(ComClient_t *comClient);

//去初始化COM实例
int gf_com_exit(ComClient_t *comClient);

char * gf_com_pids_status();

//向指定客户端发送数据
bool gf_com_send_data(ComClient_t *comClient, unsigned char *data, int size);

//长时间收到错误数据时，重启接收客户端
int gf_com_set_reboot(ComClient_t *comClient);

//供组播接收程序接收数据测试使用
int gf_com_push_data(unsigned char *buf, int nread);
#ifdef __cplusplus
}
#endif

#endif
