/**
 * @file   swhttpserver.h
 * @author Hongchen Dou
 * @date   Sun Jul 15 21:57:13 2007
 * 
 * @brief  http server
 * 
 * 
 */

#ifndef __SWHTTPSERVER_H__
#define __SWHTTPSERVER_H__

#include "swtype.h"

typedef enum
{
  HTTP_OK = 200,
  HTTP_MOVED_TEMPORARILY = 302,
  HTTP_BAD_REQUEST = 400,       /* malformed syntax */
  HTTP_UNAUTHORIZED = 401, /* authentication needed, respond with auth hdr */
  HTTP_NOT_FOUND = 404,
  HTTP_FORBIDDEN = 403,
  HTTP_REQUEST_TIMEOUT = 408,
  HTTP_NOT_IMPLEMENTED = 501,   /* used for unrecognized requests */
  HTTP_INTERNAL_SERVER_ERROR = 500,
#if 1 /* future use */
  HTTP_CONTINUE = 100,
  HTTP_SWITCHING_PROTOCOLS = 101,
  HTTP_CREATED = 201,
  HTTP_ACCEPTED = 202,
  HTTP_NON_AUTHORITATIVE_INFO = 203,
  HTTP_NO_CONTENT = 204,
  HTTP_MULTIPLE_CHOICES = 300,
  HTTP_MOVED_PERMANENTLY = 301,
  HTTP_NOT_MODIFIED = 304,
  HTTP_PAYMENT_REQUIRED = 402,
  HTTP_BAD_GATEWAY = 502,
  HTTP_SERVICE_UNAVAILABLE = 503, /* overload, maintenance */
  HTTP_RESPONSE_SETSIZE=0xffffffff
#endif
} HttpResponseNum;

#define HTTPSERVER_MAX_LINE 1024

#ifndef HTTP_AUTHENTICATION_REQUIRED
#define HTTP_AUTHENTICATION_REQUIRED
#endif

typedef struct
{
	char method[8];
	char request_url[1024];
	char host[128];
	char accept_type[16];
	char accept_encoding[16];
	char connection[16];
#ifdef HTTP_AUTHENTICATION_REQUIRED
	char authorization[256];
#endif	
	char content_type[16];
	int content_length;
	int header_length;
}HTTP_REQUEST_HEADER;

	
typedef struct
{
	/* 工作套接字 */
	int skt;
	/* 客户端ip，网络字节序 */
	unsigned long from_ip;
	/* 客户端端口 */
	unsigned short from_port;
	/* 服务器句柄 */
	HANDLE hHttpServer;
	/* 工作缓冲区 */
	HTTP_REQUEST_HEADER request_header;
	char buf[HTTPSERVER_MAX_LINE];
}SHttpConnectObj;


#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * 定义HttpServer回调函数
 * 触发时机： 接收一个新的连接时
 * 
 * @param obj
 * 接收新连接，同时产生一个连接实例(内部分配内存，需要手工调用sw_httpserver_close_connectobj释放）
 * @param wparam
 * 附加参数
 * @return 
 */
typedef int (*PHttpServerCallback)(SHttpConnectObj* obj, uint32_t wparam); 

/** 
 * 开启一个httpserver 
 * 
 * @param port 服务器端口号， 网络序
 * @param callback 回调函数
 * @param wparam  回调函数的参数
 * 
 * @return 返回Httpserver句柄
 */
HANDLE sw_httpserver_open(unsigned short port, PHttpServerCallback callback, uint32_t wparam );

/** 
 * 关闭httpserver
 * 
 * @param server 
 */
void sw_httpserver_close( HANDLE server );

/** 
 * 接收Request报头， 成功则填充request_header结构
 * 
 * @param obj 
 * @param timeout 
 * 
 * @return int: 成功返回报文头的大小， 失败返回负数
 */
int sw_httpserver_recv_request_header(SHttpConnectObj* obj, int timeout );
	
/** 
 * 接收请求实体
 * 
 * @param obj 
 * @param buf 
 * @param buf_size 
 * @param timeout 
 * 
 * @return 返回接收数据的字节数， 0: 接收完成， -1: 接收出错
 */
int sw_httpserver_recv_request_content(SHttpConnectObj* obj, char*buf, int buf_size, int timeout );


/** 
 * 
 * 发送响应报文头
 * @param obj 
 * @param responseNum 
 * @param pContentType 
 * @param pContentEncoding 
 * @param pConnection 
 * @param nContentLength 
 * @param timeout 
 * 
 * @return 发送的数据字节数
 */
int sw_httpserver_send_response_header(SHttpConnectObj* obj, HttpResponseNum responseNum, 
									   char* pContentType, char* pContentEncoding, 
									   char* pConnection,
									   int nContentLength, 
									   int timeout);
	
/** 
 * 发送响应实体
 * 
 * @param obj 
 * @param buf 
 * @param buf_size 
 * @param timeout 
 * 
 * @return 返回发送数据字节数 -1：发送失败
 */
int sw_httpserver_send_response_content(SHttpConnectObj* obj,
											char*buf, int buf_size,
											int timeout);

/** 
 * 关闭连接,同时释放资源
 * 
 * @param obj 
 */
void sw_httpserver_close_connectobj(SHttpConnectObj* obj);
	
#ifdef __cplusplus
}
#endif

#endif
