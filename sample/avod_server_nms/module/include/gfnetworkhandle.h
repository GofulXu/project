#ifndef __GFNETWORKHANDLE_H__
#define __GFNETWORKHANDLE_H__

#ifdef __cplusplus
extern "C"  {
#endif

#define GFNETWORKHANDLE_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "networkhandle", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFNETWORKHANDLE_LOG_INFO( format, ...)		gf_log(LOG_LEVEL_INFO, "networkhandle", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFNETWORKHANDLE_LOG_WARN( format, ...)		gf_log(LOG_LEVEL_WARN, "networkhandle", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFNETWORKHANDLE_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "networkhandle", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFNETWORKHANDLE_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "networkhandle", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)


//接收回调函数类型
typedef bool (*PReportHandler)( unsigned long ip, unsigned short port, char *buf, int size);

/***********************************
 *网络模块初始化
 *注:未调用gf_networkhandle_param_init(), 则使用默认参数配置
 *return 0 suc, -1 err
 *
 * ***********************/
int gf_networkhandle_init(void);

/***********************************
 *网络模块退出函数
 *
 * ***********************/
void gf_networkhandle_exit(void);

/**********************************
 *设置接收回调函数
 *注:para init 后设置
 *PReportHandler pReportHandler;接收回调函数指针
 *return 0 suc -1 err
 */
int gf_networkhandle_setcallback(PReportHandler pReportHandler);

#ifdef __cplusplus
}
#endif
#endif /*__GFNETWORKHANDLE_H__*/
