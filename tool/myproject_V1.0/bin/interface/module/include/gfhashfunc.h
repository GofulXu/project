#ifndef __GFHASHFUNC_H__
#define __GFHASHFUNC_H__

/*****************************
 *使用gfhashfunc模块,整个程序退出控制由exitfun注册控制;
 *
 *
 * *****************************/

#ifdef __cplusplus
extern "C"{
#endif

#define GFHASHFUNC_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "gfhashfunc", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFHASHFUNC_LOG_INFO( format, ...) 	gf_log(LOG_LEVEL_INFO, "gfhashfunc", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFHASHFUNC_LOG_WARN( format, ...) 	gf_log(LOG_LEVEL_WARN, "gfhashfunc", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFHASHFUNC_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "gfhashfunc", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFHASHFUNC_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "gfhashfunc", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

//gf	hash	函数指针定义
typedef int (*hash_fun_ptr)(char *, char *, int);

void gf_printf_uthash_function( );
int gf_clearuthash_function( );
int gf_delete_uthash_function(char *name);
int gf_douthash_function(char *cmd, char *value, char *re_value, int re_size);
int gf_hash_adduthash_function(char *name, hash_fun_ptr mfun);


#ifdef __cplusplus
}
#endif

#endif /*__GFHASHFUNC_H*/
