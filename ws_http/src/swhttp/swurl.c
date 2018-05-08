/***************************************************************************
* AUTHOR:  
* CONTENT: 实现URL分析功能
* NOTE:	
* HISTORY:
* [2005-10-05] created
****************************************************************************/
#include "swapi_linux.h"
#include "swtype.h"
#include "swurl.h"
#include "txtparser.h"


/* 分析url到destURL结构中,0表示分析成功,-1分析失败 */
int sw_url_parse(SURL* dst, char* url)
{
	char* p = NULL;
	char* q = NULL;
	char* r = NULL;
	char* s = NULL;

	int i=0,j=0;
	
	/* find head */
	memset(dst->head,0,sizeof(dst->head));
	p = strchr( url,':');
	if( p != NULL && (*(p+1)=='/' || *(p+1)=='\\') )
	{
		q = p -1;
		for(;q>url;q--)
		{
			if( !( (*q>='a' && *q<='z') || (*q>='A' && *q<='Z') || (*q>='0' && *q<='9') ) ) 
			{
				q++;
				break;	
			}
		}
		for( i=0;i<sizeof(dst->head)-1 && q<p;i++,q++)
			dst->head[i]= *q;	
		
		dst->head[i]='\0';
		
		/* skip ':' */
		p++;
		/* skip "// " */
		for(i=0;i<2;i++)
		{
			if( *p=='/' || *p=='\\' )	
				p++;
		}	
	}
	else
	{
		p = url;
	}
	/* find user name & password */
	memset(dst->user,0,sizeof(dst->user));
	memset(dst->pswd,0,sizeof(dst->pswd));

	q = strchr( p,'@');
	r = strchr( p, '/' );
	if( r == NULL )
		r = strchr( p, '\\' );
	if( q != NULL && (r==NULL || q<r) )
	{
		i=0;j=-1;
		for( ;p<q; p++)
		{
			if( i<sizeof( dst->user)-1 && j < 0 && *p!=':')
				dst->user[i++] = *p;
			else if( j >= 0 && j< sizeof( dst->pswd)-1 )
				dst->pswd[j++] = *p;
			else if( *p == ':')
				j=0;
		}
		p++;
	}
	
	/* find hostname & port	 */
	memset(dst->hostname,0,sizeof(dst->hostname));
	i = 0;
	q = url + strlen(url);
	for( ;p<q; )
	{
		/* 找header */
		if( *p == ':' || *p=='/' || *p=='\\')
		{
			/* find port */
			if( *p == ':' )
			{
				/* skip it */
				p++;
				dst->port =(uint16_t)strtol(p,&q,0);
				p = q;
			}
			else
				dst->port = 0;
			break;
		}
		else if( i< sizeof(dst->hostname)-1 ) 
		{
			dst->hostname[i] = *p;  	
			i++;
			p++;
		}	
		else
		{
			i=0;
		}
	}
	dst->hostname[i] = '\0';	
	
	/* find path  */
	memset(dst->path,0,sizeof(dst->path));
	i = 0;
	q = url + strlen(url);
	r = NULL;
	for( ;p<q && i<sizeof(dst->path)-1;p++)
	{
		dst->path[i] = *p; 	
		if( *p == '?' || *p ==';')
			r = p;
		else if( !r && ( *p=='/' || *p =='\\') )
			s =p;
			
		i++;
	}
	dst->path[i] = '\0';
	if( !r )
		r = q;		
		
	/* find tail & suffix from path */
	memset(dst->tail,0,sizeof(dst->tail));
	memset(dst->suffix,0,sizeof(dst->suffix));

	i=0;
	j=0;

	p = s;
	q = r;
	if( p != NULL )
	{
		/* skip '/' or '\\' */
		p++;
		for(;p<=q ;p++)
		{
			if( *p != '?' &&  *p != ';' && *p != '\0' ) 
			{
				if( i < sizeof(dst->tail)-1 )
				{
					dst->tail[i]= *p ;
					i++;	
				}
								
				if( *p == '.' || j>0 )
				{
				  if(j>0 && *p == '.')
				    {
				      j = 0;
				      dst->suffix[j] = *p;
				    }
				  if( j < sizeof(dst->suffix)-1 )
					{
						dst->suffix[j] = *p;
						j++;
					}
				    
				}
			}
			else
				break;
		}
		
		dst->tail[i] = '\0';
		dst->suffix[j] = '\0';
	}
	
	/* 重新转换ip和port */
	if( strlen(dst->hostname) >0 )
	{
		
		if( !IsAddress(dst->hostname) ) 
		{
#ifdef VXWORKS		
			dst->ip = hostGetByName( dst->hostname);
			if( dst->ip != ERROR )
			{
				char szBuf[INET_ADDR_LEN];
				memset( (void*)szBuf, 0, sizeof(szBuf) );
				inet_ntoa_b( *((struct in_addr*)&(dst->ip)), szBuf );
				hostAdd( dst->hostname, szBuf );
			}
#else 
			struct hostent *h = gethostbyname(dst->hostname);
			if( h!= NULL )
				memcpy(&(dst->ip), h->h_addr_list[0], sizeof(dst->ip));			
#endif
		}
		else
			
			dst->ip = inet_addr(dst->hostname);
	}
	/* SURL端口为网络序 */
	dst->port =  htons(dst->port);
	
	return 0;
}

/* 取得URL中的参数 */
char* sw_url_get_param_value( char* url, char* name, char *value, int valuesize )
{
	int i=0;
	char *p;

	p = strstr( url, name );
	/* 确认找到name参数 */
	while( p != NULL )
	{
		/* param name 前面必须是分界符,后面必须是等号 */
		if( ( p == url  || ( p > url && 
			( *(p-1) == '?' || *(p-1) == ';' || *(p-1) == '&' || *(p-1) == ',') ) )
			&&  *(p+strlen(name)) == '=' )	 
		{
			/* 定位到'=' */
			p = p + strlen(name);
			/* skip '=' */
			p++;
			
			/* 到下一个分界符,认为是结束 */
			i=0;
			for(;;p++)
			{
				if( *p==';' || *p=='&' || *p==',' || *p=='\0' )
				{
					break;
				}
				if( i < valuesize-1 )
				{
					value[i] = *p;
					i++;
				}
				else
					break;
			} 
			value[i] = '\0';
			return value;
		}
   		else
		    p = p + strlen(name);
		
		/* 没有找到param name,继续向后找param name */	
		p = strstr(p,name);
	}
	
	return NULL;
}

/* 取得URL中的参数 */
char*  sw_url_get_param(char* url,char* name)
{
	static char value[128];
	if( sw_url_get_param_value( url, name, value, sizeof(value) ) )
		return value;
	else
		return NULL;
}

/* 从URL中提取整数 */
int sw_url_get_param_int( char* url, char* name )
{
	char* p = sw_url_get_param( url, name );
	if( p && p[0] )
		return atoi(p);
	else
		return -1;
}


/* 对URL进行编码 */
int sw_url_encode(char* in, char* out)
{
	int i = 0;
	char table[] ={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	int count = 0;
	int len = strlen(in);
	for(i=0;i<len;i++)
	{
	 	if( in[i]>=0x21 && in[i]<=0x7E && in[i]!='<' && in[i]!='>' && in[i]!='^' && in[i]!='[' 
			 && in[i]!=']' && in[i]!='{' && in[i]!='}' && in[i]!='%' )
		{
			out[count++] = in[i];
		}
		else
		{
	    	out[count++]='%';
	    	out[count++]=table[ ((unsigned char)in[i])/16];
	    	out[count++]=table[ ((unsigned char)in[i])%16];
		}
	}
	out[count++]='\0';
	return strlen(out);
}

#if 0
int main( int argc, char *argv[] )
{

  SURL url;
  sw_url_parse(&url, argv[1]);

/*   sw_url_get_param(argv[1], "DRM"); */

/*   getchar(); */
/*   sw_url_parse(&url, argv[1]); */

/*   printf ("tail=%s\n", url.tail); */
  return 0;
}

#endif
