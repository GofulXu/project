/* 
 * @file gftime.h
 * @brief 本地时间模块
 * @author liuguodong
 * @date 2014-12-02
 */
#ifndef gfTIME_H
#define gfTIME_H


	
#ifdef __cplusplus
extern "C"
{
#endif

#define GFTIME_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "gftime", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFTIME_LOG_INFO( format, ...) 	gf_log(LOG_LEVEL_INFO, "gftime", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFTIME_LOG_WARN( format, ...) 	gf_log(LOG_LEVEL_WARN, "gftime", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFTIME_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "gftime", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFTIME_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "gftime", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

int gf_time_init();

int gf_time_get_time();

char* gf_time_get_strtime();

int gf_time_adjust_time(int year, int mon, int day, int hour, int min, int sec);

int gf_time_adjust_strtime(const char *strtime);

#ifdef __cplusplus
}
#endif

#endif
