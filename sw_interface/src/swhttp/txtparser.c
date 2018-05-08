/**
 * CONTENT: 对文本格式进行分析
 * HISTORY：
 *		[2005-10-18] chenkai created
 */
#include "swapi_linux.h" 
#include <ctype.h>
#include "txtparser.h"
#include "swlog.h"
#include "swdefine.h"
#include "string_ext.h"

/* 分析是不是一个有效的点分十进制IP地址 */
bool IsAddress(char* buf)
{
	int i = 0, len = 0;
	char *p = NULL, *pSep = NULL;
	int iCount = 0;
	char szTemp[8];
	int nSect = 0;

	if( buf == NULL )
		return false;
		
	len = strlen(buf);
	if( len == 0 || len > 15 )
		return false;

	for( i = 0; i < len; i++)
	{
		if( ( buf[i] < '0' || buf[i] >'9' ) && ( buf[i] != '.' ) )
			return false;
	}

	p = buf;
	while( ( pSep = strchr(p,'.') ) != NULL )
	{
		++iCount;

		if( pSep - p > 3)
			return false;

		memset( (void*) szTemp, 0, sizeof(szTemp) );
		strncpy( szTemp, p, pSep - p );

		nSect = atoi( szTemp );
		if( nSect > 255 )
			return false;

		pSep++;
		p = pSep;
	}

	if( iCount != 3 )
	{
		return false;
	}
	else
	{
		memset( (void*) szTemp, 0, sizeof(szTemp) );
		strcpy( szTemp, p);

		nSect = atoi( szTemp );
		if( nSect > 255 )
			return false;
	}
	return true;	
}

/* 提取等式的左右两边的字符串 */
char* equation_parse(char* equations,char** pleft,char** pright)
{
	char* bptr = equations;
/* 	去掉空格 */
	ADV_SPACE(bptr);
/* 	取得等式的左边 */
		*pleft = bptr; 	
/* 	找等于号 */
	while( *bptr !='\0' && *bptr !=';' && *bptr !='&' )  
	{
		if( *bptr == '=')
		 	break;
		bptr++;
	}
/* 	找到等于号,取等式右边 */
	if( *bptr == '=' )
	{
		bptr++;
/* 		去掉空格 */
		ADV_SPACE(bptr);
		*pright = bptr;
	}	
	else
		*pright = NULL;
	
/* 	找到下一个equatio的开始 */
	while (*bptr != '\0' && *bptr != ';' && *bptr != '&') 
		bptr++;
		
	if (*bptr == ';' || *bptr == '&') 
		bptr++;
	
	return bptr;
}

/* 提取等式的左右两边的字符串 */
char* equation_parse_as_line(char* equations,char** pleft,char** pright)
{
	char* bptr = equations;

	//去掉特殊字符和空格
	while( *bptr !='\0' && (*bptr =='\r' || *bptr =='\n' || *bptr ==' ') )
		bptr++;
	
	//取得等式的左边
	*pleft = bptr; 	
	//找等于号
	while( *bptr !='\0' && *bptr !='\r' && *bptr !='\n' )  
	{
		if( *bptr == '=')
		 	break;
		bptr++;
	}
	//找到等于号,取等式右边
	if( *bptr == '=' )
	{
		bptr++;
		//去掉空格
		while(*bptr == ' ')
			bptr ++;
		*pright = bptr;
	}	
	else
		*pright = NULL;
	
	//找到下一个equatio的开始
	while (*bptr != '\0' && *bptr !='\r' && *bptr !='\n') 
		bptr++;

	return bptr;
}

static unsigned char to_hex (char *ptr)
{
	if (isdigit(*ptr)) 
	{
		return (*ptr - '0');
	}
	return (tolower(*ptr) - 'a' + 10);
}

/* 把数字字符串转化为2进制数组 */
int  txt2hex(const char* string, int length,uint8_t* binary,int binsize)
{
	char* iptr = NULL;
	int len;
	uint8_t* bptr = NULL;
	int ret = -1;
	
	iptr = (char*)string;
	len=0;
	while ( isxdigit(*iptr) && len<length) 
	{
		iptr++;
		len++;
	}
	len >>= 1;
	if (len == 0 )
	{
		sw_log_debug(  "Error in fmtp config statement!");
		return -1;
	}
	
	if( binsize < len)
	{
		sw_log_debug("Buffer not enought!");
		return -1;
	}

	bptr = binary;

	ret = len;  
	iptr =(char*)string; 
	while (len > 0) 
	{
		*bptr++ = (to_hex(iptr) << 4) | (to_hex(iptr + 1));
		iptr+=2;
		len--;
	}
	return ret;
}
/* 是否是一个整数 */
bool IsInt( char* srcint )
{
	int aint = atoi( srcint );
	
	char buf[64];
	memset( buf, 0, sizeof(buf) );
	sprintf( buf, "%d", aint );
	if( xstrcasecmp( srcint, buf ) == 0 )
	{
		return true;
	}

	return false;
}
/* 参数有效性检查 是否是Mac */
bool IsMacaddress( char* srcMac )
{
	int i = 0;
	if( strlen( srcMac ) == 17 )
	{
		int flag = 2;
		for( i = 0; i<17; i++ )
		{
			if( flag > 0 )
			{
				if( srcMac[i] < '0' || 'f' < srcMac[i] )
					return false;
			}
			else
			{
				if( srcMac[i] != ':' )
					return false;
				flag = 2;
				continue;
			}
			flag --;
		}
		return true;
	}

	return false;
}
/* 参数有效性检查 是否是带宽 */
bool IsBand( char* srcBand )
{
	int i = 0;
	char bandname[][16] = 
		{
			"256k",		//0x0   
			"512k",		//0x1
			"1M",		//0x2
			"2M",		//0x3
			"5M",		//0x4
			"10M",		//0x5
			"20M",		//0x6
			"50M",		//0x7
			"Unlimited",//0x8   //带宽控制不起作用
			"Enable",	//0x9
			"Disable"	//0xa
		};

	for( ; i<sizeof(bandname)/sizeof(bandname[0]); i++ )
	{
		if( xstrcasecmp( srcBand, bandname[i] ) == 0 )
			return true;
	}

	sw_log_debug( "invalid netband [%s]\n", srcBand );
	return false;
}

/* 参数有效性检查 是否是一个port */
bool IsPort( char* srcport )
{
	unsigned short port = atoi( srcport );
	char buf[16];
	memset( buf, 0, sizeof(buf) );
	sprintf( buf, "%d", port );
	if( xstrcasecmp( srcport, buf ) == 0 )
	{
		return true;
	}
	sw_log_debug( "[%s] error port\n", srcport );
	return false;
}

/* 参数有效性检查 是否是音量值 */
bool IsVol( char* srcVol )
{
	int aint = atoi( srcVol );
	
	char buf[64];
	memset( buf, 0, sizeof(buf) );
	sprintf( buf, "%d", aint );
	if( xstrcasecmp( srcVol, buf ) == 0 )
	{
		if( aint >= 0 && aint <=31 )
			return true;
		else
			sw_log_debug( "volume must between 0 and 31\n" );
		return false;
	}
	
	sw_log_debug( "volume must be a integer\n" );
	return false;
}


/* 参数有效性检查 是否是netmode */
bool IsNetmode(char* srcNetmode)
{
	if( xstrcasecmp(srcNetmode,"static") == 0 || xstrcasecmp(srcNetmode,"dhcp") == 0
		|| xstrcasecmp(srcNetmode,"auto-static") == 0 || xstrcasecmp( srcNetmode,"pppoe")== 0 )
		return true;
	else
		return false;
}
