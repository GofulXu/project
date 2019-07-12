#ifndef __GFparamerter_H__
#define __GFparamerter_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define PARAM_PATH	"./general.db"

#define GFPARAM_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "gfparam", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFPARAM_LOG_INFO( format, ...) 	gf_log(LOG_LEVEL_INFO, "gfparam", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFPARAM_LOG_WARN( format, ...) 	gf_log(LOG_LEVEL_WARN, "gfparam", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFPARAM_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "gfparam", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFPARAM_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "gfparam", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)


//数据库参数表初始化
int gf_paramerter_init();

//数据库参数表退出处理
void gf_paramerter_exit();

//获取参数string
int gf_paramerter_get(char *name, char *value, int size);

//获取参数int
int gf_paramerter_get_int(char *name);

//通用表插入新参数
int gf_paramerter_insert(char *name, char *value);

//系统表插入新参数
int gf_paramerter_insert_system(char *name, char *value);

//删除参数
int gf_paramerter_delete(char *name);

//设置参数string
int gf_paramerter_set(char *name, char *value);

//设置参数int
int gf_paramerter_set_int(char *name, int value);

#ifdef __cplusplus
}
#endif

#endif /*__GFparamerter_H__*/
