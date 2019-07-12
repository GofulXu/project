#ifndef __GFCOMHANDLE_H__
#define __GFCOMHANDLE_H__


#ifdef __cplusplus
extern "C"  {
#endif

#define GFCOMHANDLE_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "comhandle", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFCOMHANDLE_LOG_INFO( format, ...) 	gf_log(LOG_LEVEL_INFO, "comhandle", __FUNCTION__, __LINE__, format, ##__VA_ARGS__) 
#define GFCOMHANDLE_LOG_WARN( format, ...) 	gf_log(LOG_LEVEL_WARN, "comhandle", __FUNCTION__, __LINE__, format, ##__VA_ARGS__) 
#define GFCOMHANDLE_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "comhandle", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFCOMHANDLE_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "comhandle", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

/***********************************
 *模块初始化
 *注:未调用gf_com_param_init(), 则使用默认参数配置
 *return 0 suc, -1 err
 *
 * ***********************/
int gf_com_handle_init(void);

/***********************************
 *网络模块退出函数
 *
 * ***********************/
int gf_com_handle_exit(void);

#ifdef __cplusplus
}
#endif
#endif /*__GFCOMHANDLE_H__*/
