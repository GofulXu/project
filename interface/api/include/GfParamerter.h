#ifndef __GFparamerter_H__
#define __GFparamerter_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define PARAM_PATH	"./general.db"

#define GFPARAM_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "GfParam", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFPARAM_LOG_INFO( format, ...) 	gf_log(LOG_LEVEL_INFO, "GfParam", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFPARAM_LOG_WARN( format, ...) 	gf_log(LOG_LEVEL_WARN, "GfParam", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFPARAM_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "GfParam", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFPARAM_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "GfParam", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)


//数据库参数表初始化
int GfParamerterInit();

//数据库参数表退出处理
void GfParamerterExit();

//获取参数string
int GfParamerterGet(char *Name, char *Value, int Size);

//获取参数int
int GfParamerterGetInt(char *Name);

//通用表插入新参数
int GfParamerterInsert(char *Name, char *Value);

//删除参数
int GfParamerterDelete(char *Name);

//设置参数string
int GfParamerterSet(char *Name, char *Value);

//设置参数int
int GfParamerterSetInt(char *Name, int Value);

#ifdef __cplusplus
}
#endif

#endif /*__GFparamerter_H__*/
