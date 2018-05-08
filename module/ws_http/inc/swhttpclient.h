/** 
 * @file swhttpclient.h
 * @brief HTTP函数接口
 * @author NiuJiuRu
 * @date 2007-10-29
 */
#include "swtype.h"

#ifndef __SWHTTPCLIENT_H__
#define __SWHTTPCLIENT_H__
#define HTTP_OK 0
#define HTTP_ERROR_PORT_NORESPONSE 1
#define HTTP_ERROR_PORT_DENY  2 
#define HTTP_ERROR_HTTP_NORESPONSE 3
#define HTTP_ERROR_HTTP_NORESPONSE_404 404
#define HTTP_ERROR_HTTP_NORESPONSE_302 302
#define HTTP_ERROR_HTTP_NORESPONSE_403 403
#define HTTP_ERROR_HTTP_NORESPONSE_500 500
#define HTTP_ERROR_HTTP_NORESPONSE_206 206
#define HTTP_ERROR_HTTP_NORESPONSE_301 301
#define HTTP_ERROR_HTTP_NORESPONSE_400 400
#define HTTP_ERROR_HTTP_NORESPONSE_406 406


/*
 *	HTTP客户端
 */
typedef struct
{
	/* 工作套接字 */
	int skt;
	/* 服务器ip，网络字节序 */
	unsigned long ip;
	/* 服务器端口 */
	unsigned short port;
	char szCookies[1024];
	char version[8];
	/* 占用标志 */
	int used;
}SHttpClient;


/*
 *	HTTP摘要认证
 */
typedef struct _HTTP_AUTHENTICATION_ {
	char arithmetic[10];		// 算法名称: md5, md5-sess
	char user_name[16];		// 用户名 
	char user_pwd[16];		// 密码
	char realm[32];			// realm name
	char nonce[48];			// 服务器随机产生的nonce返回串
	char uri[256];			// 请求URL
	char qop[10];			// qop-value: "", "auth", "auth-int"
	char opaque[48];		// opaque value
}SHttpAuthentcation;

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief 与Http服务器建立连接
 * 
 * @param ip http服务器地址
 * @param port http服务器端口
 * @param timeout 
 * 
 * @return 还回连接成功后的句柄
 */
HANDLE sw_httpclient_connect( unsigned long ip, unsigned short port, int timeout );

/** 
 * @brief 断开与Http服务器建立的连接
 * 
 * @param hHttpclient 连接成功后的句柄
 */
void sw_httpclient_disconnect( HANDLE hHttpclient );

/** 
 * @brief 发送HTTP Request
 * 
 * @param hHttpclient 建立连接后的句柄
 * @param pMethod 请求方式
 * @param pURL 请求的URL
 * @param pHost 主机名称
 * @param pAcceptType 接收的文件类型
 * @param pContentType 内容类型
 * @param pContent 内容
 * @param nLength 内容长度
 * @param timeout 超时
 * @param pSOAPAction: soap action URI
 * @param pHttpAuth 摘要认证(NULL, 不使用摘要认证; NOT NULL, 要使用的摘要认证信息)
 * 
 * @return 请求是否发送成功
 */
bool sw_httpclient_request( HANDLE hHttpclient, char* pMethod, char* pURL, char *pHost,
			char *pAcceptType, char* pContentType, char* pContent, int nLength, int timeout,char* pSOAPAction,SHttpAuthentcation* pHttpAuth );

/** 
 * @brief 接收HTTP数据
 * 
 * @param hHttpclient 建立连接后的句柄
 * @param pBuf 接收缓冲区
 * @param size 缓冲区大小
 * @param timeout 接收超时
 * 
 * @return 接收到的数据长度
 */
int sw_httpclient_recv_data( HANDLE hHttpclient, char* pBuf, int size, int timeout );

/** 
 * @brief 取得负载长度
 * 
 * @param pHeadBuf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 
 * @return 
 */
int sw_httpclient_get_content_size(char* pHeadBuf, int size);

/** 
 * @brief 取得返回值
 * 
 * @param pHeadBuf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 
 * @return 
 */
int sw_httpclient_get_ret_code(char* pHeadBuf, int size);

/** 
 * @brief 得到http header的长度
 * 
 * @param pHeadBuf HTTP报文头接收
 * @param size 缓冲区大小
 * 
 * @return 
 */
int sw_httpclient_get_header_size(char* pHeadBuf, int size);

/** 
 * @brief 注册cookies
 * 
 * @param hHttpclient 已连接客户端的句柄
 * @param cookies 预设置的cookies存放缓冲区
 * 
 * @return 
 */
int sw_httpclient_register_cookies(HANDLE hHttpclient, char* cookies);

/** 
 * @brief 设置HTTP版本号
 * 
 * @param hHttpclient 已连接客户端的句柄
 * @param version 预设置的HTTP版本号
 * 
 * @return 
 */
void sw_httpclient_set_version( HANDLE hHttpclient, char* version );

void sw_httpclient_set_err_code(int code);
int sw_httpclient_get_err_code( );

#ifdef __cplusplus
}
#endif

#endif /* __SWHTTPCLIENT_H__ */

