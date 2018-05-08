/** 
 * @file swhttpclient.h
 * @brief HTTP函数接口
 * @author NiuJiuRu
 * @date 2007-10-29
 */
//#include "stdafx.h"
#include "swapi_linux.h"
#include "swhttpclient.h"
#include "swtcp.h"
#include "swurl.h"
#include "swlog.h"
#include "swmutex.h"
#include "swthrd.h"
#include "http_authentication.h"
#include "swtype.h"
#include "swdefine.h"
//#include "string_ext.h"

#define MAX_HTTPCLIENT_NUM		32


static SHttpClient m_all[MAX_HTTPCLIENT_NUM];
static int m_ref = -1;
static HANDLE m_mutex = NULL;

/* 全局Cookies */
static char m_szCookies[1024] = {0,};
static int m_error_code=HTTP_OK;

/***********************************************************************************************
* CONTENT: 与Http服务器建立连接
* PARAM:
	[in] ip: http服务器地址
	[in] port: http服务器端口
* RETURN:
	还回连接状态
* NOTE:
************************************************************************************************/
HANDLE sw_httpclient_connect( unsigned long  ip, unsigned short port, int timeout )
{
	SHttpClient* pHttpclient = NULL; 
	unsigned long unblock = 1;
	fd_set wset, rset;
	int i, n;
	int retcode;
	unsigned int now;
	static bool first = true;	
	struct linger lingerOpt;	
	lingerOpt.l_onoff = 1;
  lingerOpt.l_linger = 0;

	if(	m_mutex == NULL)	
		m_mutex = sw_mutex_create();

	if(m_mutex)
		sw_mutex_lock(m_mutex);
	if( m_ref < 0 )
	{
		m_ref = 0;

		memset( m_all, 0, sizeof(m_all) );
		for( i=0; i<MAX_HTTPCLIENT_NUM; i++ )
			m_all[i].skt = -1;
	}
	for( i=0; i<MAX_HTTPCLIENT_NUM; i++ )
	{
		if( m_all[i].used == 0 )
		{
			m_all[i].used = 1;
			pHttpclient = m_all + i;
			break;
		}
	}
	m_ref++;

	m_error_code=HTTP_OK;	

	if(m_mutex)
		sw_mutex_unlock(m_mutex);

	sw_log_debug("[HTTPCLIENT]0x%x, ip=%d, port=%d, timeout=%d, ref=%d\n",pHttpclient, ip, port, timeout, m_ref);

	if( pHttpclient == NULL )
	{
		sw_httpclient_set_err_code( HTTP_ERROR_HTTP_NORESPONSE );
		goto ERROR_EXIT;
	}

	strcpy(pHttpclient->version, "1.0");
	pHttpclient->ip = ip;
	pHttpclient->port = port;
	
	if(strlen(m_szCookies) > 0)
	  strncpy(pHttpclient->szCookies, m_szCookies, sizeof(pHttpclient->szCookies));
	else
	  memset( pHttpclient->szCookies, 0, sizeof(pHttpclient->szCookies) );

	/* 创建socket */
	pHttpclient->skt = sw_tcp_socket();
	if( pHttpclient->skt < 0 )
	{
		sw_httpclient_set_err_code( HTTP_ERROR_HTTP_NORESPONSE );
		goto ERROR_EXIT;
	}
	/* 配置为非阻塞 */
	if( sw_tcp_ioctl( pHttpclient->skt, FIONBIO, &unblock ) < 0 )
	{
		sw_httpclient_set_err_code( HTTP_ERROR_HTTP_NORESPONSE );
		goto ERROR_EXIT;
	}
	{
		int reuse = 1;
		setsockopt(pHttpclient->skt , SOL_SOCKET, SO_REUSEADDR, 
				   (char *)&reuse, sizeof(reuse));	
		setsockopt( pHttpclient->skt, SOL_SOCKET, SO_LINGER,(void*) &lingerOpt, sizeof(lingerOpt) );
	}

	now = sw_thrd_get_tick();

	if( first )
	{
		srand( now );
		sw_tcp_bind(pHttpclient->skt, INADDR_ANY, htons( (unsigned short) (30000 + rand()%26000 )));
		first = false;
	}
	/* 连接... */
	errno = 0;
	retcode = sw_tcp_connect( pHttpclient->skt, ip, port );
	if(retcode == 0)
		return pHttpclient;
	if(errno != 0 && errno != EINPROGRESS)
	{
		sw_httpclient_set_err_code(HTTP_ERROR_PORT_DENY);
		sw_log_error ("[HTTPCLIENT] connect error:%d ret=%d\n", retcode, errno);
		goto ERROR_EXIT;
	}
	
	n=0;
WAIT:
	/* 等待连接成功 */
	if( (retcode = sw_tcp_select( pHttpclient->skt, &rset, &wset, NULL, timeout )) < 0 )
	{
		sw_httpclient_set_err_code( HTTP_ERROR_HTTP_NORESPONSE );
		goto ERROR_EXIT;
	}

	if(retcode == 0)
	{
		sw_log_error("[HTTPCLIENT]select socket timeout!\n");
		sw_httpclient_set_err_code(HTTP_ERROR_PORT_NORESPONSE);
		goto ERROR_EXIT;
	}

	if( FD_ISSET( pHttpclient->skt, &rset ) )
	{
		char szBuf[16];
		int readsize = sw_tcp_recv( pHttpclient->skt, szBuf, sizeof(szBuf) );
		if( readsize==-1 && (errno==EWOULDBLOCK || errno==EINPROGRESS) )
		{
			if( ++n<3 )
				goto WAIT;
		}
	}
	else if( FD_ISSET( pHttpclient->skt, &wset ) )
	{
		int err;
		int len = sizeof(err);
#ifndef WIN32
		if (getsockopt(pHttpclient->skt, SOL_SOCKET, SO_ERROR, &err, (socklen_t *)&len) < 0) 
#else
		if (getsockopt(pHttpclient->skt, SOL_SOCKET, SO_ERROR, (char*)&err, &len) < 0) 
#endif
		{
			sw_log_error ("[HTTPCLIENT]getsocketopt failed!\n");
			sw_httpclient_set_err_code( HTTP_ERROR_HTTP_NORESPONSE );
			goto ERROR_EXIT;
		}

		if (err) {
			sw_httpclient_set_err_code( HTTP_ERROR_HTTP_NORESPONSE );
			printf ("[HTTPCLIENT] connect failed erron:%d!\n", err);
			goto ERROR_EXIT;
		}
		return pHttpclient ;
	}
	sw_log_info("http client connect server %d %d \n",retcode,errno);
 	if(n==2)
		sw_httpclient_set_err_code(HTTP_ERROR_PORT_NORESPONSE);
	else
	{
		if(errno==111)
			sw_httpclient_set_err_code(HTTP_ERROR_PORT_DENY);
		else
			sw_httpclient_set_err_code(HTTP_ERROR_PORT_NORESPONSE);
	}
	sw_log_debug("http client connect server ,err code %d \n",m_error_code);
ERROR_EXIT:
	if(m_mutex)
		sw_mutex_lock(m_mutex);
	if(pHttpclient )
	{
		if( 0 <= pHttpclient->skt )
		{
			sw_tcp_close( pHttpclient->skt );
			pHttpclient->skt = -1;
		}
		pHttpclient->used = 0;
	}
	m_ref--;

	if(m_mutex)
		sw_mutex_unlock(m_mutex);
	return NULL;
}


/***********************************************************************************************
* CONTENT: 断开与Http服务器建立的连接
* PARAM: 要断开的套接字  
* RETURN:
	连接服务器是否成功
* NOTE:
************************************************************************************************/
void sw_httpclient_disconnect( HANDLE hHttpclient )
{
	SHttpClient* pHttpclient = ( SHttpClient* )hHttpclient; 
	if(m_mutex)
		sw_mutex_lock(m_mutex);
	if( pHttpclient )
	{				
		if( 0 <= pHttpclient->skt )
		{
			sw_tcp_close( pHttpclient->skt );
			pHttpclient->skt = -1;
		}
		pHttpclient->used = 0;
		m_ref--;
	}
	if(m_mutex)
		sw_mutex_unlock(m_mutex);
}

/***********************************************************************************************
* CONTENT: 发送HTTP Request
* PARAM:
	[in] skt:建立连接后的套接字 
	[in] pMethod: 请求方式
	[in] pURL:请求的URL
	[in] pHost:主机名称
	[in] pAcceptType:接收的文件类型
	[in] pContentType:post 的内容类型
	[in] pContent:post 的内容
	[in] nLength: post 内容长度
	[in] pSOAPAction: soap action URI
	[in] pHttpAuth: 摘要认证信息
* RETURN:
	请求是否发送成功
* NOTE:
************************************************************************************************/

bool sw_httpclient_request( HANDLE hHttpclient, char* pMethod,char* pURL,char *pHost,
			char *pAcceptType,char* pContentType,char* pContent,int nLength, int timeout,char* pSOAPAction,SHttpAuthentcation* pHttpAuth)
{
	fd_set wset, rset, eset;	
	char* pBuf = NULL;
	char m_pReqBuf[8192];
	SHttpClient* pHttpclient = ( SHttpClient* )hHttpclient; 
	HASHHEX HA1;
	HASHHEX HA2 = "";
	HASHHEX m_response;
	char m_cNonce[48];
	char m_szNonceCount[9] = "00000001";
	
	if( !pHttpclient )
		return false;
		
	memset(m_pReqBuf,0,sizeof(m_pReqBuf));
	pBuf =  m_pReqBuf;

	memset( m_cNonce, 0, sizeof(m_cNonce) );

	/* method */
	strcpy( pBuf, pMethod );
	pBuf += strlen(pMethod);
	strcpy( pBuf, " " );
	pBuf++; 
	
	/* url */
	sw_url_encode(pURL,pBuf);
	pBuf += strlen(pBuf);
	/* 版本 */
	strcpy( pBuf, " HTTP/1.1");
	pBuf += strlen(pBuf);
	
	/* host information */
	if(pHost && strlen(pHost)>0 )
	{
		sprintf(pBuf,"\r\nHost: %s", pHost);
		pBuf += strlen(pBuf);
	}
	
	/* 接收类型 */
	strcpy( pBuf, "\r\nAccept: ");
	pBuf += strlen(pBuf);
	if( pAcceptType && strlen(pAcceptType)>0 )
	{
		strcpy( pBuf, pAcceptType );
	}
	else
	{
		strcpy( pBuf, "*/*");
	}
	pBuf += strlen(pBuf);
	
	/* soap action */
	if ( pSOAPAction != NULL ) 
	{
		if( strlen(pSOAPAction) > 0 )
			sprintf( pBuf, "\r\nSOAPAction: %s",pSOAPAction);
		else
			sprintf( pBuf, "\r\nSOAPAction:");		
		pBuf += strlen(pBuf);
	}
	
	/* 添加摘要认证信息 */
	if ( pHttpAuth != NULL ) 
	{
		// USERNAME
		strcpy( pBuf, "\r\nAuthorization: Digest username=\"");
		pBuf += strlen(pBuf);
		strcpy( pBuf, pHttpAuth->user_name );
		pBuf += strlen(pBuf);

		// REALM
		strcpy( pBuf, "\", realm=\"");
		pBuf += strlen(pBuf);
		strcpy( pBuf, pHttpAuth->realm );
		pBuf += strlen(pBuf);

		// NONCE
		strcpy( pBuf, "\", nonce=\"");
		pBuf += strlen(pBuf);
		strcpy( pBuf, pHttpAuth->nonce );
		pBuf += strlen(pBuf);

		// URI
		strcpy( pBuf, "\", uri=\"");
		pBuf += strlen(pBuf);
		strcpy( pBuf, pHttpAuth->uri );
		pBuf += strlen(pBuf);

		// QOP
		strcpy( pBuf, "\", qop=\"");
		pBuf += strlen(pBuf);
		strcpy( pBuf, pHttpAuth->qop );
		pBuf += strlen(pBuf);

		// NC
		strcpy( pBuf, "\", nc=");
		pBuf += strlen(pBuf);
		strcpy( pBuf, m_szNonceCount );
		pBuf += strlen(pBuf);

		// CNONCE
		srand( (unsigned)time( NULL ) + rand()*2 );
		sprintf( m_cNonce, "%d", rand() ) ;
		//sprintf( m_cNonce, "%s", pHttpAuth->nonce );

		strcpy( pBuf, ", cnonce=\"");
		pBuf += strlen(pBuf);
		strcpy( pBuf, m_cNonce );
		pBuf += strlen(pBuf);

		// RESPONSE
		DigestCalcHA1(pHttpAuth->arithmetic, pHttpAuth->user_name, pHttpAuth->realm, pHttpAuth->user_pwd, pHttpAuth->nonce, m_cNonce, HA1);
		DigestCalcResponse(HA1, pHttpAuth->nonce, m_szNonceCount, m_cNonce, pHttpAuth->qop, pMethod, pHttpAuth->uri, HA2, m_response);

		strcpy( pBuf, "\", response=\"");
		pBuf += strlen(pBuf);
		strcpy( pBuf, m_response );
		pBuf += strlen(pBuf);
		
		// OPAQUE
		strcpy( pBuf, "\", opaque=\"");
		pBuf += strlen(pBuf);
		strcpy( pBuf, pHttpAuth->opaque );
		pBuf += strlen(pBuf);
		*pBuf++ = '\"';
	}
	
	/* post content */
	if( strcmp( pMethod, "POST" ) == 0 )
	{
		if( nLength >0 )
		{
			sprintf( pBuf, "\r\nContent-Length: %d", nLength );
		}
		else
		{
			strcpy( pBuf, "\r\nContent-Length: 0" );
		}
		pBuf += strlen(pBuf);
		if( nLength >0 )
		{ 
			sprintf( pBuf, "\r\nContent-Type: %s", pContentType );
			pBuf += strlen(pBuf);
		}
	}
	else if( strcmp( pMethod, "GET" ) == 0 )
	{
		if( nLength >0 )
		{
			sprintf( pBuf, "\r\nRange: bytes=%d-", nLength );
			pBuf += strlen(pBuf);
		}
	}
	/* 接收语言以及编码 */
	strcpy(pBuf,"\r\nAccept-language: zh-cn\r\nAccept-Encoding: gzip, deflate");
	pBuf += strlen(pBuf);
	
	/* user-agent */
	strcpy(pBuf,"\r\nUser-Agent: Mozilla/4.0 (compatible; MS IE 6.0; EIS iPanel 2.0;(ziva))");
	pBuf += strlen(pBuf);

	strcpy(pBuf,"\r\nPragma: no-cache");
	pBuf += strlen(pBuf);	
	
	strcpy(pBuf,"\r\nCache-Control: no-cache");
	pBuf += strlen(pBuf);

	if( strlen(pHttpclient->szCookies) > 0 )
	{
		if( pBuf + strlen(pHttpclient->szCookies) - m_pReqBuf < sizeof(m_pReqBuf) - 64 )
		{
			sprintf(pBuf,"\r\n%s", pHttpclient->szCookies);
			pBuf += strlen(pBuf);
		}
	}
	
	if(xstrcasecmp(pHttpclient->version, "1.0") == 0)
		strcpy(pBuf,"\r\nConnection: close\r\n\r\n");
	else
		strcpy(pBuf,"\r\nConnection: Keep-Alive\r\n\r\n");
	pBuf += strlen(pBuf);

	/* 发送content */
/* 	if( strcmp( pMethod, "POST" ) == 0 ) */
/* 	{ */
/* 		strncpy( pBuf, pContent, nLength ); */
/* 		pBuf += nLength; */
/* 	} */
	/* 检测状态 */
	sw_tcp_select( pHttpclient->skt, &rset, &wset, &eset, timeout );
	/* 发送数据 */
	if( FD_ISSET( pHttpclient->skt, &wset ) )
	{
		/* 发送Http头 */
		if( sw_tcp_send( pHttpclient->skt, m_pReqBuf, pBuf-m_pReqBuf ) < 0 )
			return false;
		
		/* 发送content edit by xupan*/
		if( strcmp( pMethod, "POST" ) == 0 )
		{
			char *p = NULL;
			int sd = 0 ;
			p = pContent;
			int rst = 0;
			while( sd <  nLength)
			{			

ReSend:				
				if( sw_tcp_send( pHttpclient->skt, p++, 1 ) < 0 )
				{	
					sw_log_info("======= HTTP POST ERROR ========!!!!!\n\n");
					p--;
					rst ++;
					printf("lsq<<<<<<<<<socket error:%d\n",errno);
					if(rst >= 5)
						return false;
					else
						goto ReSend;	
				}
				else
				{
					rst = 0; 
					sd ++ ;	
				}
			}
		}
		return true;
	}
	return false;
}

/***********************************************************************************************
* CONTENT: 接收HTTP数据
* PARAM:
	[in] hHttpclient:建立连接后的套接字
	[in] pBuf:接收缓冲区
	[in] pHost:缓冲区大小
	[in] timeout:接收超时
* RETURN:
	接收到的数据长度
* NOTE:
************************************************************************************************/
int sw_httpclient_recv_data( HANDLE hHttpclient, char* pBuf, int size, int timeout )
{
	SHttpClient* pHttpclient = ( SHttpClient* )hHttpclient;
	fd_set rset;

	if( pHttpclient && (pHttpclient->skt != -1) )
	{
		/* 检测状态 */
		if( sw_tcp_select( pHttpclient->skt, &rset, NULL, NULL, timeout ) < 0 )
			return 0;

		/* 接收数据 */
		if( FD_ISSET(pHttpclient->skt, &rset) )
			return sw_tcp_recv( pHttpclient->skt, pBuf, size );
		else
			return 0;
	}
	return -1;
}

/** 
 * @brief 取得返回值
 * 
 * @param pHeadBuf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 
 * @return 
 */
int sw_httpclient_get_ret_code(char* pHeadBuf, int size)
{
	int i;
	char szBuffer[20];

	char* buf = pHeadBuf;
	if( pHeadBuf==NULL )
		return -1;

	if( !xstrncasecmp(buf, "http/", 5) )
	{
		buf += 5;
		while( (*buf>='0' && *buf<='9') || (*buf=='.') )
			buf++;
	}

	while( *buf==' ' )
		buf++;

	for(i=0; i<10 && *buf>='0' && *buf<='9'; i++, buf++)
	{
		szBuffer[i] = *buf;
	}
	szBuffer[i] = 0;
	return atoi( szBuffer );
}

/** 
 * @brief 取得负载长度
 * 
 * @param pHeadBuf HTTP报文头接收缓冲区
 * @param size 缓冲区大小
 * 
 * @return 
 */
int sw_httpclient_get_content_size(char* pHeadBuf, int size)
{
	int i=0;
	while( i < size )
	{
		if( !memcmp(pHeadBuf+i, "Content-Length:", 15) )
		{
			return atoi( pHeadBuf+i+15 );
		}
		while( i<size && pHeadBuf[i]!='\n' )
			i++;
		i++;
	}
	return 0;

}

/** 
 * @brief 得到http header的长度
 * 
 * @param pHeadBuf HTTP报文头接收
 * @param size 缓冲区大小
 * 
 * @return 
 */
int sw_httpclient_get_header_size(char* pHeadBuf, int size)
{
  	int i=0;
	int j=0;
	//句子编号
	int index =0;
	//句子长度
	int sentencelen=0;
	//所有句子的总长度
	int headerlen = 0;
	i=0;
	index =0;
	headerlen = 0;	
	while( i<size )
	{
		j=i;
		sentencelen = 0;
		for(;i<(size-1);i++)
		{
			if( pHeadBuf[i]=='\r' &&  pHeadBuf[i+1]=='\n' )
			{
				sentencelen = i-j+2;			
				break;
			}
		}
		
		i += 2;

		index++;
		//printf("Find  line %d length:%d i:%d size:%d\n",index,sentencelen,i,size);
		if( sentencelen >0 )
		{
			headerlen += sentencelen;
			if( sentencelen <= 2 )
			{
				break;
			}
		}
		else
			headerlen = size;
	}
	
	return headerlen;
}

/** 
 * @brief 注册cookies
 * 
 * @param hHttpclient 已连接客户端的句柄
 * @param cookies 预设置的cookies存放缓冲区
 * 
 * @return 
 */
int sw_httpclient_register_cookies(HANDLE hHttpclient,char* cookies)
{

	if(hHttpclient == NULL)
	{
		memset(m_szCookies, 0, sizeof(m_szCookies));
		if(cookies)
			strncpy(m_szCookies, cookies, sizeof(m_szCookies));
		return 0;
	}

    memset( ((SHttpClient*)hHttpclient)->szCookies, 0, sizeof(((SHttpClient*)hHttpclient)->szCookies) );
	if( cookies )
	{
		strncpy( ((SHttpClient*)hHttpclient)->szCookies, cookies, sizeof(((SHttpClient*)hHttpclient)->szCookies)-1 );
	}

	return 0;
}

/** 
 * @brief 设置HTTP版本号
 * 
 * @param hHttpclient 已连接客户端的句柄
 * @param version 预设置的HTTP版本号
 * 
 * @return 
 */
void sw_httpclient_set_version(HANDLE hHttpclient, char* version)
{
	SHttpClient* pHttpclient = ( SHttpClient* )hHttpclient; 
	if(pHttpclient)
	{
		strncpy(pHttpclient->version, version, sizeof(pHttpclient->version));
		
	}
}

int sw_httpclient_get_err_code( )
{
	return m_error_code ;
}

void sw_httpclient_set_err_code(int code)
{
	sw_log_debug("sw_httpclient_set_err_code %d\n",code);
	m_error_code=code;
}

