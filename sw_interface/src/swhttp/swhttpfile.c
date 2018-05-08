#include "swapi_linux.h"
#include "swhttpclient.h"
#include "string_ext.h"
#include "swmem.h"
#include "swthrd.h"
#include "swurl.h"
#include "swhttpfile.h"
#include "string_ext.h"
#include "swlog.h"
#include "swmutex.h"
#include "swdefine.h"

#define MAX_HTTFILE_NUM		16

/* 从HTTP头中获取返回值 */
static int sw_httpfile_find_ret_code( char* pBuf, int size );
static int bcd2str( char* buf );


typedef struct
{
	HANDLE hClient; 
	/* 文件大小 */
	int filesize;
	/* 接收缓冲区 */
	unsigned char buf[2048];
	/* 重定向URL */
	char redirect_url[2048];
	/* 占用标志 */
	int  used;
}SHttpFile;

static SHttpFile m_all[MAX_HTTFILE_NUM];
static int m_ref = -1;
static HANDLE m_mutex = NULL;
static int m_error_code=0;

/* 初始化httpfile */
HANDLE sw_httpfile_init( char *url, int timeout )
{
	SHttpFile* pHttpFile = NULL; 
	int size, recvsize;
	int retcode;
	SURL surl;
	unsigned char *p;
	int i, n, repeat;
	char value[256], cookie[512], url2[2048];
	char host_buf[256];
	
	sw_log_debug( "sw_httpfile_init(\"%s\")\n", url );
	if(	m_mutex == NULL)	
		m_mutex = sw_mutex_create();

	if(m_mutex)
		sw_mutex_lock(m_mutex);

	if( m_ref < 0 )
	{
		m_ref = 0;
		memset( m_all, 0, sizeof(m_all) );
	}
	for( i=0; i<MAX_HTTFILE_NUM; i++ )
	{
		if( m_all[i].used == 0 )
		{
			m_all[i].used = 1;
			pHttpFile = m_all + i;
			break;
		}
	}
	m_ref++;
	if(m_mutex)
		sw_mutex_unlock(m_mutex);

	sw_log_debug("[HTTPFILE]: 0x%x, m_ref:%d timeout:%d\n", pHttpFile, m_ref, timeout);
	
	if( pHttpFile == NULL )
		goto ERROR_EXIT;
	*cookie = 0;

	repeat = 0;
REDO:
	/* 分析URL */
	sw_url_parse( &surl, (char*)url );
	if( surl.port == 0 )
		surl.port = htons(80);
	/* 与服务器建立连接	 */  
	sw_log_debug("[surl.ip=%x] %x\n",surl.ip,surl.port);
	if(surl.ip ==0)
		goto ERROR_EXIT;
	pHttpFile->hClient = sw_httpclient_connect( surl.ip, surl.port, timeout );	

	if( pHttpFile->hClient == NULL )
		goto ERROR_EXIT;
	if( *cookie )
		sw_httpclient_register_cookies( pHttpFile->hClient, cookie );

	/* 发送Get请求 */
	sw_log_debug( "sw_httpclient_request 1 surl.ip=%x,surl.port=%x \n",  surl.ip,surl.port);
	snprintf(host_buf, sizeof(host_buf), "%s:%d", surl.hostname, ntohs(surl.port));

	if( ! sw_httpclient_request( pHttpFile->hClient ,"GET",
			surl.path, host_buf, NULL, NULL, NULL, 0, timeout,NULL,NULL) )
	{
		sw_httpclient_set_err_code(  HTTP_ERROR_HTTP_NORESPONSE);
		goto ERROR_EXIT;
	}
  
	/* 读取文件头用于获取文件信息 */
	size = recvsize = 0;
	while( recvsize < (int)sizeof(pHttpFile->buf)-1 )
	{
		size = sw_httpclient_recv_data( pHttpFile->hClient, (char*)pHttpFile->buf+recvsize, 1, timeout );
		if( size <= 0 )
			break;
		recvsize += size;
		p = pHttpFile->buf + recvsize - 4;
		if( 4 < recvsize && p[0]=='\r' && p[1]=='\n' && p[2]=='\r' && p[3]=='\n' )
		{
			break;
		}
	}
	pHttpFile->buf[recvsize] = 0;

	retcode = sw_httpfile_find_ret_code( (char*)pHttpFile->buf, recvsize );
	pHttpFile->filesize = sw_httpclient_get_content_size( (char*)pHttpFile->buf, recvsize );
	/* 重定向 */
	if( retcode/100 == 3 )
	{
		if(pHttpFile->hClient)
		{
			sw_httpclient_disconnect(pHttpFile->hClient);
			pHttpFile->hClient = NULL;
		}

		i = 0;

		/* 提取Location字段值 */
		memset( value, 0, sizeof(value) );
sw_log_debug( "sw_httpfile_init redirect %s\n", pHttpFile->buf );
		if( xstrgetval( (char*)(pHttpFile->buf), "Location", value, sizeof(value) ) )
			i += sprintf( url2+i, "%s", value );
		else
			goto ERROR_EXIT;
		/* 提取cookie字段值 */
#if 0
		/* 测试表明， Set-Cookie 字段可以设置多个变量，变量之间用分号隔开。这就不能直接复制 */
		memset( value, 0, sizeof(value) );
		if( xstrgetval( (char*)(pHttpFile->buf), "Set-Cookie", value, sizeof(value) ) )
		{
			char seps[] = ";";

			n = 0;
			p = strtok(value, seps);
			while( p )
			{
				while( *p == ' ' || *p == '\r' || *p == '\n' )
					p++;
				if( n == 0 )
					i += sprintf( url2+i, "?%s", p );
				else
					i += sprintf( url2+i, "&%s", p );
				p = strtok(NULL, seps);
				n++;
			}
		}
#else
		*cookie = 0;
		if( xstrgetval( (char*)(pHttpFile->buf), "Set-Cookie", cookie+7, sizeof(cookie)-7 ) )
			memcpy( cookie, "Cookie:", 7 );
#endif

		//对于url2，进行BCD码转换
		bcd2str( url2 );
		sw_log_debug( "redirect url: %s\n", url2 );

		if( !strcmp(url, url2) )		/* 测试发现，url能重定向到自身 */
			repeat += 5;
		if( ++repeat>20 )
			goto ERROR_EXIT;
		strcpy( pHttpFile->redirect_url, url2 );
		url = pHttpFile->redirect_url;
		goto REDO;
	}

	if( retcode<=0 || retcode >=400 )
	{
		sw_httpclient_set_err_code( retcode);
		sw_log_debug( "Download url not exist (%s) retcode=%d\n", url, retcode );
		goto ERROR_EXIT;
	}
	sw_httpclient_set_err_code( HTTP_OK);
	return pHttpFile;

ERROR_EXIT:

	sw_log_debug( "error: sw_httpfile_init(\"%s\") failed\n", url );

	if( pHttpFile )
	{
		if( pHttpFile->hClient )
			sw_httpclient_disconnect( pHttpFile->hClient );
		if(m_mutex)
			sw_mutex_lock(m_mutex);
		memset( pHttpFile, 0, sizeof(*pHttpFile) );
		if(m_mutex)
			sw_mutex_unlock(m_mutex);
	}
	m_ref--;
	
	return NULL;
}

/* 初始化httpfile */
HANDLE sw_httpfile_addl_init( char *url, char *cookie, int timeout )
{
	SHttpFile* pHttpFile = NULL; 
	int size, recvsize;
	int retcode;
	SURL surl;
	unsigned char *p;
	int i, n, repeat;
	char value[256], url2[2048];
	char host_buf[256];
	
	sw_log_debug( "sw_httpfile_init(\"%s\")\n", url );
	if(	m_mutex == NULL)	
		m_mutex = sw_mutex_create();

	if(m_mutex)
		sw_mutex_lock(m_mutex);

	if( m_ref < 0 )
	{
		m_ref = 0;
		memset( m_all, 0, sizeof(m_all) );
	}
	for( i=0; i<MAX_HTTFILE_NUM; i++ )
	{
		if( m_all[i].used == 0 )
		{
			m_all[i].used = 1;
			pHttpFile = m_all + i;
			break;
		}
	}
	m_ref++;
	if(m_mutex)
		sw_mutex_unlock(m_mutex);

	sw_log_debug("[HTTPFILE]: 0x%x, m_ref:%d timeout:%d\n", pHttpFile, m_ref, timeout);
	
	if( pHttpFile == NULL )
		goto ERROR_EXIT;

	repeat = 0;
REDO:
	/* 分析URL */
	sw_url_parse( &surl, (char*)url );
	if( surl.port == 0 )
		surl.port = htons(80);
	/* 与服务器建立连接	 */  
	sw_log_debug("[surl.ip=%x] %x\n",surl.ip,surl.port);
	if(surl.ip ==0)
		goto ERROR_EXIT;
	pHttpFile->hClient = sw_httpclient_connect( surl.ip, surl.port, timeout );	

	if( pHttpFile->hClient == NULL )
		goto ERROR_EXIT;
	if( *cookie )
		sw_httpclient_register_cookies( pHttpFile->hClient, cookie );

	/* 发送Get请求 */
	sw_log_debug( "sw_httpclient_request 1 surl.ip=%x,surl.port=%x \n",  surl.ip,surl.port);
	snprintf(host_buf, sizeof(host_buf), "%s:%d", surl.hostname, ntohs(surl.port));

	if( ! sw_httpclient_request( pHttpFile->hClient ,"GET",
			surl.path, host_buf, NULL, NULL, NULL, 0, timeout,NULL,NULL) )
	{
		goto ERROR_EXIT;
	}
  
	/* 读取文件头用于获取文件信息 */
	size = recvsize = 0;
	while( recvsize < (int)sizeof(pHttpFile->buf)-1 )
	{
		size = sw_httpclient_recv_data( pHttpFile->hClient, (char*)pHttpFile->buf+recvsize, 1, timeout );
		if( size <= 0 )
			break;
		recvsize += size;
		p = pHttpFile->buf + recvsize - 4;
		if( 4 < recvsize && p[0]=='\r' && p[1]=='\n' && p[2]=='\r' && p[3]=='\n' )
		{
			break;
		}
	}
	pHttpFile->buf[recvsize] = 0;

	retcode = sw_httpfile_find_ret_code( (char*)pHttpFile->buf, recvsize );
	pHttpFile->filesize = sw_httpclient_get_content_size( (char*)pHttpFile->buf, recvsize );
	/* 重定向 */
	if( retcode/100 == 3 )
	{
		if(pHttpFile->hClient)
		{
			sw_httpclient_disconnect(pHttpFile->hClient);
			pHttpFile->hClient = NULL;
		}

		i = 0;

		/* 提取Location字段值 */
		memset( value, 0, sizeof(value) );
sw_log_debug( "sw_httpfile_init redirect %s\n", pHttpFile->buf );
		if( xstrgetval( (char*)(pHttpFile->buf), "Location", value, sizeof(value) ) )
			i += sprintf( url2+i, "%s", value );
		else
			goto ERROR_EXIT;
		/* 提取cookie字段值 */
#if 0
		/* 测试表明， Set-Cookie 字段可以设置多个变量，变量之间用分号隔开。这就不能直接复制 */
		memset( value, 0, sizeof(value) );
		if( xstrgetval( (char*)(pHttpFile->buf), "Set-Cookie", value, sizeof(value) ) )
		{
			char seps[] = ";";

			n = 0;
			p = strtok(value, seps);
			while( p )
			{
				while( *p == ' ' || *p == '\r' || *p == '\n' )
					p++;
				if( n == 0 )
					i += sprintf( url2+i, "?%s", p );
				else
					i += sprintf( url2+i, "&%s", p );
				p = strtok(NULL, seps);
				n++;
			}
		}
#else
		*cookie = 0;
		if( xstrgetval( (char*)(pHttpFile->buf), "Set-Cookie", cookie+7, sizeof(cookie)-7 ) )
			memcpy( cookie, "Cookie:", 7 );
#endif

		//对于url2，进行BCD码转换
		bcd2str( url2 );
		sw_log_debug( "redirect url: %s\n", url2 );

		if( !strcmp(url, url2) )		/* 测试发现，url能重定向到自身 */
			repeat += 5;
		if( ++repeat>20 )
			goto ERROR_EXIT;
		strcpy( pHttpFile->redirect_url, url2 );
		url = pHttpFile->redirect_url;
		goto REDO;
	}

	if( retcode<=0 || retcode >=400 )
	{
		sw_log_debug( "Download url not exist (%s) retcode=%d\n", url, retcode );
		goto ERROR_EXIT;
	}

	return pHttpFile;

ERROR_EXIT:

	sw_log_debug( "error: sw_httpfile_init(\"%s\") failed\n", url );

	if( pHttpFile )
	{
		if( pHttpFile->hClient )
			sw_httpclient_disconnect( pHttpFile->hClient );
		if(m_mutex)
			sw_mutex_lock(m_mutex);
		memset( pHttpFile, 0, sizeof(*pHttpFile) );
		if(m_mutex)
			sw_mutex_unlock(m_mutex);
	}
	m_ref--;
	
	return NULL;
}

/* 退出 */
void sw_httpfile_exit( HANDLE hFile, int timeout )
{
	SHttpFile* pHttpFile = (SHttpFile*)hFile;

	if( pHttpFile && pHttpFile->hClient )
	{
		sw_httpclient_disconnect( pHttpFile->hClient );
		if(m_mutex)
			sw_mutex_lock(m_mutex);
		memset( pHttpFile, 0, sizeof(*pHttpFile) );
		m_ref--;
		if(m_mutex)
			sw_mutex_unlock(m_mutex);
	}
}

/* 取得HTTP服务器上文件的大小 */
int sw_httpfile_get_size( HANDLE hFile )
{
	SHttpFile* pHttpFile = (SHttpFile*)hFile;
	return  pHttpFile ? pHttpFile->filesize : 0;
}

/* 获取服务器文件 */
int sw_httpfile_get_file( HANDLE hFile, char *buf, int size, int timeout )
{
	int bufpos = 0, recvsize = 0;
	SHttpFile* pHttpFile = (SHttpFile*)hFile;

	while( bufpos < size && bufpos < pHttpFile->filesize )
	{
		recvsize = sw_httpclient_recv_data( pHttpFile->hClient, buf+bufpos, size-bufpos, timeout );
		if( recvsize <= 0 )
			break;

		bufpos += recvsize;
	}
	return bufpos; 		
}


int sw_httpfile_allocmem_getfile( HANDLE hFile, char **pBuf, int *pSize, int timeout )
{
	SHttpFile* pHttpfile = (SHttpFile*)hFile;
	char *buf;
	int size, recvsize;
	int err = 0;

	struct _SMemNode
	{
		char buf[64*1024];
		int  size;
		struct _SMemNode *next;
	}*head, *tail, *p;

	*pBuf = NULL;
	*pSize = 0;

	if( pHttpfile==NULL )
	{
		return -1;
	}

	if( pHttpfile->filesize > 0 )
	{
		buf = (char*)malloc( pHttpfile->filesize + 1 );
		if( buf==NULL )
		{
			return -4;
		}
		size = sw_httpfile_get_file( hFile, buf, pHttpfile->filesize, timeout );
		*pBuf = buf;
		*pSize = size;
		if( size < pHttpfile->filesize )
			return -3;
		buf[size] = 0;
		return 0;
	}

	head = NULL;
	size=0;
	while( 1 )
	{
		p = (struct _SMemNode*)malloc( sizeof(struct _SMemNode) );
		if( p==NULL )
		{
			printf( "HttpFile_AllocMemGetFile: not enough memory!\n" );
			err = -2;
			break;
		}
		p->size=0;
		p->next=NULL;
		while( p->size < sizeof(p->buf) )
		{
			recvsize = sw_httpclient_recv_data( pHttpfile->hClient, p->buf+p->size, sizeof(p->buf)-p->size, timeout );
			if( recvsize <= 0 )
				break;
			p->size += recvsize;
		}
		if( head==NULL )
			head=p;
		else
			tail->next=p;
		tail=p;
		size += p->size;
		if( recvsize<0 )
		{
			err = -3;
			break;
		}
		if( recvsize==0 )
			break;
	}

	buf = NULL;
	if( size > 0 && err==0 )
	{
		pHttpfile->filesize = size;
		buf = (char*)malloc( pHttpfile->filesize + 1 );
		if( buf == NULL )
		{
			err = -4;
			printf( "HttpFile_AllocMemGetFile: OOM!\n" );
		}
		else
		{
			recvsize=0;
			for( p=head; p; p=p->next )
			{
				memcpy( buf+recvsize, p->buf, p->size );
				recvsize += p->size;
			}
			buf[recvsize]=0;
		}
	}
	for( p=head; p; p=tail )
	{
		tail = p->next;
		free(p);
	}

	*pBuf = buf;
	*pSize = size;

	return buf||err ? err : -5;
}




/* 取得实际使用的URL，因为有可能重定向了*/
char *sw_httpfile_get_url( HANDLE hFile )
{
	return ((SHttpFile*)hFile)->redirect_url;
}

/* 取得httpclient句柄 */
HANDLE sw_httpfile_get_client( HANDLE hFile )
{
	return ((SHttpFile*)hFile)->hClient;
}

/* 从HTTP头中获取返回值 */
static int sw_httpfile_find_ret_code( char* pBuf, int size )
{
	int i;
	char szBuffer[20];

	char* buf = pBuf;
	if( pBuf==NULL )
		return -1;

	if( !xstrncasecmp(buf, "http/", 5) )
	{
		buf += 5;
		while( (*buf>='0'&&*buf<='9') || (*buf=='.') )
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

#define IsHex(c)	('0'<=(c) && (c)<='9' || 'a'<=(c) && (c)<='f' || 'A'<=(c) && (c)<='F')
#define Char2Hex(c) ('0'<=(c) && (c)<='9' ? ((c)&0xf) : ((c)&0xf)+9)

static int bcd2str( char* buf )
{
	int i, j;
	j = 0;
	for( i=0; buf[i]; i++ )
	{
		if( buf[i]=='%' && IsHex(buf[i+1]) && IsHex(buf[i+2]) )
		{
			buf[j] = (Char2Hex(buf[i+1])<<4) | Char2Hex(buf[i+2]);
			i += 2;
		}
		else if( i!=j )
		{
			buf[j] = buf[i];
		}
		j++;
	}
	buf[j] = 0;
	return j;
}

