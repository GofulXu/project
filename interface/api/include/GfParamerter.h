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
int GfParamerterGet(const char *Name, char *Value, int Size);

//获取参数int
int GfParamerterGetInt(const char *Name);

//通用表插入新参数
int GfParamerterInsert(const char *Name, char *Value);

//删除参数
int GfParamerterDelete(const char *Name);

//设置参数string
int GfParamerterSet(const char *Name, char *Value);

//设置参数int
int GfParamerterSetInt(const char *Name, int Value);

int GfDeviceGet(const char *Uuid, char *UserId, int IdSize, char *PassWd, int WdSize, char *UserName, int NaSize, int *Type);
int GfDeviceInsert(const char *Uuid, char *UserId, char *PassWd, char *UserName, int Type);
int GfDeviceDelete(const char *Uuid);
int GfDeviceSet(const char *Uuid, char *PassWd, char *UserName);
#ifdef __cplusplus
}
#endif

#endif /*__GFparamerter_H__*/
