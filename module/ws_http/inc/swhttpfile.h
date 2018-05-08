/** 
 * @file swhttpfile.h
 * @brief HTTP方法下载文件
 * @author ...
 * @date 2007-09-06
 */

#ifndef __SWHTTPFILE_H__
#define	__SWHTTPFILE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief 初始化http连接下载
 * 
 * @param url 
 * @param timeout 
 * 
 * @return httpfile句柄,成功;失败为NULL
 */
HANDLE sw_httpfile_init( char *url, int timeout );

/** 
 * @brief 退出httpfile
 * 
 * @param hFile 
 * @param timeout 
 */
void sw_httpfile_exit( HANDLE hFile, int timeout );

/** 
 * @brief 打印信息
 */
void sw_httpfile_print();

/** 
 * @brief 取得httpclient句柄
 * 
 * @param hFile 
 * 
 * @return httpclient句柄
 */
HANDLE sw_httpfile_get_client( HANDLE hFile );

/** 
 * @brief 取得HTTP服务器上文件的大小
 * 
 * @param hFile 
 * 
 * @return 
 */
int sw_httpfile_get_size( HANDLE hFile );

/** 
 * @brief 获取服务器文件
 * 
 * @param hFile 
 * @param buf 
 * @param size 
 * @param timeout 
 * 
 * @return 
 */
int sw_httpfile_get_file( HANDLE hFile, char *buf, int size, int timeout );

/** 
 * @brief 取得实际使用的URL，因为有可能重定向了
 * 
 * @param hFile 
 * 
 * @return 
 */
char *sw_httpfile_get_url( HANDLE hFile );


#ifdef __cplusplus
}
#endif

#endif	/* __SWHTTPFILE_H__ */
