#include "swapi_linux.h"
#include "swhttpserver.h"
#include "swtcp.h"
#include "swmem.h"
#include "swlog.h"
#include "string_ext.h"

//#include "stdafx.h"
//#include "swtype.h"
#include "swdefine.h"
#include "swthrd.h"
typedef struct
{
	/* 工作套接字 */
	int skt;
	/* 服务器ip，网络字节序 */
	unsigned long ip;
	/* 服务器端口 */
	unsigned short port;
}SHttpServer;

typedef struct
{
	HttpResponseNum type;
	const char *name;
	const char *info;
} HttpEnumString;

static const HttpEnumString httpResponseNames[] = {
	{ HTTP_OK, "OK", "" },
	{ HTTP_MOVED_TEMPORARILY, "Found", "Directories must end with a slash." },
	{ HTTP_REQUEST_TIMEOUT, "Request Timeout",
	  "No request appeared within a reasonable time period." },
	{ HTTP_NOT_IMPLEMENTED, "Not Implemented",
	  "The requested method is not recognized by this server." },
	{ HTTP_UNAUTHORIZED, "Unauthorized", "" },
	{ HTTP_NOT_FOUND, "Not Found",
	  "The requested URL was not found on this server." },
	{ HTTP_BAD_REQUEST, "Bad Request", "Unsupported method." },
	{ HTTP_FORBIDDEN, "Forbidden", "" },
	{ HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error",
	  "Internal Server Error" },
#if 1                               /* not implemented */
	{ HTTP_CREATED, "Created" },
	{ HTTP_ACCEPTED, "Accepted" },
	{ HTTP_NO_CONTENT, "No Content" },
	{ HTTP_MULTIPLE_CHOICES, "Multiple Choices" },
	{ HTTP_MOVED_PERMANENTLY, "Moved Permanently" },
	{ HTTP_NOT_MODIFIED, "Not Modified" },
	{ HTTP_BAD_GATEWAY, "Bad Gateway", "" },
	{ HTTP_SERVICE_UNAVAILABLE, "Service Unavailable", "" },
#endif
};
static const char RFC1123FMT[] = "%a, %d %b %Y %H:%M:%S GMT";


static PHttpServerCallback m_httpserver_callback = NULL;
static unsigned long m_cb_param = 0;

static HANDLE m_hThrd = NULL;
static bool HttpServerProc( unsigned long wParam, unsigned long lParam );

/** 
 * 开启一个httpserver 
 * 
 * @param port 服务器端口号， 网络序
 * @param callback 回调函数
 * @param wparam  回调函数的参数
 * 
 * @return 返回Httpserver句柄
 */
HANDLE sw_httpserver_open(unsigned short port, PHttpServerCallback callback, uint32_t wparam )
{
	int fd;
	int on = 1;
	SHttpServer* pServer = NULL;
	
	fd = sw_tcp_socket();
	if(fd < 0)
	{
		sw_log_fatal("sw_httpserver_open::Create socket failed!\n");
		return NULL;
	}
	
#ifdef SO_REUSEPORT
	setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (void *)&on, sizeof(on)) ;
#else
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on)) ;
#endif
	sw_tcp_bind(fd, INADDR_ANY, port);
	sw_tcp_listen(fd, 5);
	if((pServer = sw_malloc(sizeof(SHttpServer))) == NULL)
	{
		sw_log_fatal("[HTTPSERVER]sw_malloc failed!\n");
		return NULL;
				
	}
	memset(pServer, 0, sizeof(SHttpServer));
	pServer->skt = fd;
	pServer->ip = INADDR_ANY;
	pServer->port = port;
	
	m_httpserver_callback = callback;
	m_cb_param = wparam;
	
	m_hThrd = sw_thrd_open( "tHttpServer", 80, 0, 16384, (PThreadHandler)HttpServerProc, (unsigned long)pServer, 0 );

	if(m_hThrd == NULL)
	{
		sw_log_fatal("[HTTPSERVER]sw_thrd_open failed!\n");
		sw_free(pServer);
		return NULL;	
	}

	sw_thrd_resume( m_hThrd );

	return pServer;
}


/** 
 * 关闭httpserver
 * 
 * @param server 
 */
void sw_httpserver_close( HANDLE server )
{
	SHttpServer* pServer = (SHttpServer*)server;
	if(m_hThrd)
	{
		sw_thrd_close(m_hThrd, 5000);
		m_hThrd = NULL;
	}
	m_httpserver_callback = NULL;
	m_cb_param = 0;
	
	sw_tcp_close(pServer->skt);
	
	sw_free(pServer);	
}

static int getLine(SHttpConnectObj* obj)
{
	int  count = 0;
	char *buf = obj->buf;

	while (sw_tcp_recv(obj->skt, buf + count, 1) == 1)
	{
		obj->request_header.header_length++;
	  
		if (buf[count] == '\r') continue;
		if (buf[count] == '\n') {
			buf[count] = 0;
			return count;
		}
		if(count < (HTTPSERVER_MAX_LINE - 1))      /* check owerflow */
			count++;
	}
	if (count) return count;
	else return -1;
}

/** 
 * 接收Request报头， 成功则填充request_header结构
 * 
 * @param obj 
 * @param timeout 
 * 
 * @return int: 成功返回报文头的大小， 失败返回负数
 */
int sw_httpserver_recv_request_header(SHttpConnectObj* obj, int timeout )
{
	char* p = NULL;
	
	memset(&obj->request_header, 0, sizeof(HTTP_REQUEST_HEADER));
	
	do
	{
		if(getLine(obj) <= 0)
			break;
		
		if(xstrncasecmp(obj->buf, "GET", strlen("GET")) == 0 || xstrncasecmp(obj->buf, "POST", strlen("POST")) == 0 )
		{
			p = strtok(obj->buf, " ");
			if(p == NULL)
				continue;
			strncpy(obj->request_header.method, p, 8);
			p = strtok(obj->buf, " ");
			if(p == NULL)
				continue;

			strncpy(obj->request_header.request_url, p, sizeof(obj->request_header.request_url));
			
		}
		else if(xstrncasecmp(obj->buf, "Host:", strlen("Host:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strncpy(obj->request_header.host, p+1, sizeof(obj->request_header.host));
			
		}
		else if(xstrncasecmp(obj->buf, "Accept:", strlen("Accept:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strncpy(obj->request_header.accept_type, p+1, sizeof(obj->request_header.accept_type));
			
		}
		else if(xstrncasecmp(obj->buf, "Accept-Encoding:", strlen("Accept-Encoding:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strncpy(obj->request_header.accept_encoding, p+1, sizeof(obj->request_header.accept_encoding));
			
		}	
		else if(xstrncasecmp(obj->buf, "Content-Type:", strlen("Content-Type:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strncpy(obj->request_header.content_type, p+1, sizeof(obj->request_header.content_type));
			
		}
		else if(xstrncasecmp(obj->buf, "Content-Length:", strlen("Content-Length:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			obj->request_header.content_length = atoi(p+1);			
		}	
		else if(xstrncasecmp(obj->buf, "Connection:", strlen("Connection:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strncpy(obj->request_header.connection, p+1,  sizeof(obj->request_header.connection));
			
		}			
#ifdef HTTP_AUTHENTICATION_REQUIRED
		else if(xstrncasecmp(obj->buf, "Authorization:", strlen("Authorization:")) == 0)
		{
			p = strchr(obj->buf, ' ');
			strncpy(obj->request_header.authorization, p+1,  sizeof(obj->request_header.authorization));
		}
#endif
	}while(1);

	sw_log_debug("[HTTPSERVER GET REQUEST]\n");
	sw_log_debug("[METHOD]:%s\n", obj->request_header.method);
	sw_log_debug("[URL]:%s\n", obj->request_header.request_url);
	sw_log_debug("[HOST]:%s\n", obj->request_header.host);
	sw_log_debug("[ACCEPT_TYPE]:%s\n", obj->request_header.accept_type);
	sw_log_debug("[CONNECTION]:%s\n", obj->request_header.connection);
	sw_log_debug("[CONTENT_TYPE]:%s\n", obj->request_header.content_type);
	sw_log_debug("[CONTENT_LENGHT]:%d\n", obj->request_header.content_length);

	return obj->request_header.header_length;	
}
	
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
int sw_httpserver_recv_request_content(SHttpConnectObj* obj, char*buf, int buf_size, int timeout )
{
	if(obj->request_header.content_length > 0)
	{
		return sw_tcp_recv(obj->skt, buf, buf_size);
	}
	return 0;
}

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
									   int timeout)
{
	char *buf = obj->buf;
	const char *responseString = "";
	const char *infoString = NULL;
	time_t timer = time(0);
	char timeStr[80];
	int i, len;

	for (i = 0; i < (sizeof(httpResponseNames)/sizeof(httpResponseNames[0])); i++) 
	{
		if (httpResponseNames[i].type == responseNum) 
		{
			responseString = httpResponseNames[i].name;
			infoString = httpResponseNames[i].info;
			break;
		}
	}

	if(infoString == NULL)
	{
		sw_log_fatal("Wrong response number[%d]\n",  responseNum);
		return -1;
		
	}
	

	/* emit the current date */
	strftime(timeStr, sizeof(timeStr), RFC1123FMT, gmtime(&timer));

	len = sprintf(buf,
				  "HTTP/1.1 %d %s\r\n"
				  "Date: %s\r\n",
				  responseNum, responseString, timeStr);
#ifdef HTTP_AUTHENTICATION_REQUIRED
	if(responseNum == HTTP_UNAUTHORIZED)
	{
		srand( (unsigned)time( NULL ) + rand()*2 );
		len += sprintf(buf+len, "WWW-Authenticate: Digest realm=\"HttpDigestAuthentication\", qop=\"auth\", nonce=\"%d\", opaque=\"abcd01082883008ab01082883008abcd\"\r\n", rand());
	}
#endif
	if(pContentType)
		len += sprintf(buf+len, "Content-type: %s\r\n", pContentType);
	if(pContentEncoding)
		len += sprintf(buf+len, "Content-Encoding: %s\r\n", pContentEncoding);
	if(nContentLength)
		len += sprintf(buf+len, "Content-Length: %d\r\n", nContentLength);
	if(pConnection)
		len += sprintf(buf+len, "Connection: %s\r\n", pConnection);
    
	len += sprintf(buf+len, "\r\n");
	
	sw_log_debug("[HTTPSERVER RESPONSE]\n%s\n", buf);
	
	i = 0;
	while(i < len)
		i += sw_tcp_send(obj->skt, buf + i, len - i);

	return len;
}
	
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
										int timeout)
{

	return sw_tcp_send(obj->skt, buf, buf_size);

}

/** 
 * 关闭连接,同时释放资源
 * 
 * @param obj 
 */
void sw_httpserver_close_connectobj(SHttpConnectObj* obj)
{
	if(obj)
	{
		sw_tcp_close(obj->skt);
		sw_free(obj);	
	}
}
	

static bool HttpServerProc( unsigned long wParam, unsigned long lParam )
{
	fd_set rset;
	unsigned long accept_ip;
	unsigned short port;
	int skt = -1;
	
	SHttpServer *pServer = (SHttpServer *)wParam;

	if(sw_tcp_select(pServer->skt, &rset, NULL, NULL, 2000) > 0)
	{
		if(FD_ISSET(pServer->skt, &rset))
		{
			if((skt = sw_tcp_accept(pServer->skt, &accept_ip, &port)) >= 0)
			{
				SHttpConnectObj* obj = sw_malloc(sizeof(SHttpConnectObj));
				int on = 1;
				
				if(obj == NULL)
				{
					sw_log_fatal("[HTTPSERVER]sw_malloc failed!\n");
					return true;
										
				}
				obj->skt = skt;
				obj->from_ip = accept_ip;
				obj->from_port = port;
				obj->hHttpServer = pServer;

				setsockopt(skt, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof (on));

				if(m_httpserver_callback)
					m_httpserver_callback(obj, m_cb_param);
				
				return true;
			}
		}
	}
	
	return true;
}
