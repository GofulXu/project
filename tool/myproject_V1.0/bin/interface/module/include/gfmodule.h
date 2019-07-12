#ifndef __GFMODULE_H__
#define __GFMODELE_H__


#ifdef __cplusplus
extern "C"  {
#endif

#define GFMODULE_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "module", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFMODULE_LOG_INFO( format, ...)		gf_log(LOG_LEVEL_INFO, "module", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFMODULE_LOG_WARN( format, ...)		gf_log(LOG_LEVEL_WARN, "module", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFMODULE_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "module", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFMODULE_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "module", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

int gf_module_init(void);

void gf_module_exit(void);


#ifdef __cplusplus
}
#endif

#endif
