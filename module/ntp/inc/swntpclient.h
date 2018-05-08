/** 
 * @file swntpclient.h
 * @brief 定义NTP client(RFC 868/958/1305/2030 Network Time Protocol)
 * @author lijian 
 * @date 2004-04-07 lijian created.
 *       2005-10-13 chenkai modifyed the function name.
 */


#ifndef __SWNTPCLIENT_H__
#define __SWNTPCLIENT_H__


#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief 初始化Client客户端
 * 
 * @param server 
 * @param timezone 单位：分钟
 * @param event_post ntp事件抛送回调函数
 * @param handler 事件抛送的句柄
 *
 * @return true,成功; false, 失败
 */
bool sw_ntpclient_init( char *server, int timezone,event_post_func event_post, void* handler);

/** 
 * @brief 释放Client客户端
 */
void sw_ntpclient_exit();

/** 
 * @brief 设置网络时钟服务器地址
 * 
 * @param server 
 */
void sw_ntpclient_set_server( char *server );
/* 
 *@brief 立即同步ntp
 * */
void sw_immediately_ntp(int timeminute);
/** 
 * @brief 得到网络时钟服务器地址
 * 
 * @return 
 */
char* sw_ntpclient_get_server();

/** 
 * @brief 设置时区
 * 
 * @param timezone 单位：分钟
 */
void sw_ntpclient_set_timezone( int timezone );

/** 
 * @brief 取得系统当前设置的时区
 * 
 * @return 单位：分钟
 */
int sw_ntpclient_get_timezone();

/** 
 * @brief 设置到NTP-SERVER上获取时间的间隔，单位：second
 * 
 * @param interval 
 */
void sw_ntpclient_set_interval( unsigned int interval ); 

/** 
 * @brief 获取到NTP-SERVER上获取时间的间隔，单位：second
 * 
 * @param interval 
 * 
 * @return 
 */
int sw_ntpclient_get_interval( unsigned int interval ); 

/** 
 * @brief 获取本地时间
 * 
 * @return 
 */
char*  sw_ntpclient_get_localtime();

/** 
 * @brief 获取ntp连接状态
 * 
 * @return true,连接; false, 未连接
 */
bool sw_ntpclient_get_connstatus();

 
void sw_ntpclient_reset_connstatus();

void sw_ntpclient_resume();

/**
 * @brief 设置夏令时的时区(分钟),开始时间,结束时间
 *
 * @param timezone 夏令时区,
		  start_time 夏令时开始时间
		  end_time 夏令时结束时间
 * @return not used
 */
int sw_ntpclient_set_sumtime(int timezone, uint32_t start_time, uint32_t end_time);

/**
 * @brief 获取夏令时的时区(分钟),开始时间,结束时间
 *
 * @param(out)  timezone 夏令时区,
		  		start_time 夏令时开始时间
		  		end_time 夏令时结束时间
 * @return 是否启用夏令时, 1,yes;0,no;
 */
int sw_ntpclient_get_sumtime(int* timezone, uint32_t* start_time, uint32_t* end_time);

#ifdef __cplusplus
}
#endif

#endif /* __NTPCLIENT_H__ */
