/** 
 * @file swurl.h
 * @brief 定义一个URL分析功能接口
 * @author sunniwell
 * @date 2005-01-05 created 
 */

#ifndef __SWURL_H__
#define __SWURL_H__


#ifdef __cplusplus
extern "C"
{
#endif
	
typedef struct _URL
{
    /** 
     * @brief URL头 '// :'之前的一个词
     */
	char head[32];	
	char user[32];
	char pswd[32];
	/** 
	 * @brief URL 域名信息(可能是IP)
	 */
	char hostname[256];
	/** 
	 * @brief URL指向的IP，网络字节序
	 */
#ifdef SUPPORT_IPV6
	struct in6_addr ipv6;
#endif
	uint32_t ip;
	/** 
	 * @brief URL端口，网络字节序
	 */
	uint16_t port;
	/** 
	 * @brief URL path
	 */
	char path[1024];		
	/** 
	 * @brief URL最后一个词
	 */
	char tail[256];
	/** 
	 * @brief URL最后一个词的后缀
	 */
	char suffix[32];	
}SURL;
/** 
 * @brief 分析URL,
 * 
 * @param dst 指向分析后的结果
 * @param url 指向的源URL
 * 
 * @return 0表示分析成功,-1分析失败
 */
int sw_url_parse(SURL* dst, char* url);


/** 
 * @brief 取得URL中的参数
 * 
 * @param url 指向的源URL
 * @param name 指向的参数名
 * @param value 指向的参数值
 * @param valuesize 指向参数值的长度
 * 
 * @return 成功,返回指向的参数值; 否则,返回NULL
 */
char* sw_url_get_param_value( char* url, char* name, char *value, int valuesize );

/** 
 * @brief 从URL中提取参数
 * 
 * @param url 指向的源URL
 * @param name 指向的参数名
 * 
 * @return 成功,返回指向的参数值; 否则,返回NULL
 */
char* sw_url_get_param(char* url,char* name);

/** 
 * @brief 从URL中提取整数
 * 
 * @param url 指向的源URL
 * @param name 指向的参数名
 * 
 * @return 成功,返回参数的整数值; 否则,返回-1
 */
int sw_url_get_param_int(char* url, char* name);

/** 
 * @brief 对URL进行编码
 * 
 * @param in 指向的输入字符串
 * @param out 指向的输出字符串
 * 
 * @return 成功,返回0; 否则,返回-1
 */
int sw_url_encode(char* in,char* out);
	
	
#ifdef __cplusplus
}
#endif

#endif /* __SWURL_H__ */
