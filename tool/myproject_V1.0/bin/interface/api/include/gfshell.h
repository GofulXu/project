#ifndef __GFSHELL_H__
#define __GFSHELL_H__

/*****************************
 *使用gfshell模块,整个程序退出控制由exitfun注册控制;
 *
 *
 * *****************************/

#ifdef __cplusplus
extern "C"{
#endif

#define GFSHELL_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "gfshell", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFSHELL_LOG_INFO( format, ...) 	gf_log(LOG_LEVEL_INFO, "gfshell", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFSHELL_LOG_WARN( format, ...) 	gf_log(LOG_LEVEL_WARN, "gfshell", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFSHELL_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "gfshell", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFSHELL_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "gfshell", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

//gf	shell	函数指针定义
typedef int (*shell_fun_ptr)(int,int);

//gf	shell	退出处理函数指针定义
typedef void (*exit_fun_ptr)(void);

//shell处理函数,阻塞等待shell命令,exit退出后,执行所有注册的exit_fun_ptr(先进后出)
void gf_shell_run();

//注册shell退出处理函数
int gf_shell_adduthash_exitfunction(exit_fun_ptr mfun);

//注册shell命令行函数
int gf_shell_adduthash_function(char *name, shell_fun_ptr mfun);

//退出shell命令行,同时退出整个程序
void gf_shell_exit(void);

#ifdef __cplusplus
}
#endif

#endif /*__GFSHELL_H*/
