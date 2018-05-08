/** 
 * @file gflog.h
 * @brief The file defines the interfaces to manipulate logging information.
 *	level 控制日志输出级别
 *	target 控制日志输出的目标，串口，文件等
 *	mods 控制日志输出的模块，把模块名称转换成32位int型数字，然后对比可以输出日志的模块的int型数组，如果在数组中则输出日志，否则不输出日志 	
 *
 * Copyright (c) 2006 Beijing Sunniwell Broadband Digital Technologies, Inc. All Rights Reserved.
 * @author Chen Guang / Hongchen Dou / huanghuaming
 * @date 2007-09-06
 * @history:
 *		2010-06-18 chenkai add log modules interface
 */

#ifndef __GFLOG_H__
#define __GFLOG_H__

/* logging level */
#define	LOG_LEVEL_ALL		0x00	//输出所有日志
#define	LOG_LEVEL_DEBUG		0x01	//指出细粒度信息事件对调试应用程序是非常有帮助的
#define	LOG_LEVEL_INFO		0x02	//消息在粗粒度级别上突出强调应用程序的运行过程
#define	LOG_LEVEL_WARN		0x03	//表明会出现潜在错误的情形
#define	LOG_LEVEL_ERROR		0x04	//指出虽然发生错误事件，但仍然不影响系统的继续运行
#define	LOG_LEVEL_FATAL		0x05	//指出每个严重的错误事件将会导致应用程序的退出
#define	LOG_LEVEL_OFF		0x06	//关闭日志输出

#ifdef __cplusplus
extern "C"
{
#endif
	
/** 
 * @brief Initialize the logging API, prepare for logging.
 * 
 * @param level int,  The original level to record the logging infomation. You can use gf_log_set_level() to change it.
 * @param target char*,  The target of all logging infomation.
 * @param mods char*,The modules which allow to ouput log.
 * 
 * @return int , The status of this operation.  0 on success, else on failure.
 */
int gf_log_init( int level, char* targets,char* mods);

/** 
 * @brief Release some resources allocated by gf_log_init().
 */
void gf_log_exit();

/** 
 * @brief set the logging level
 * 
 * @param level int, the logging level to set.
 * @return 0 is success,1 is failed.
 */
int gf_log_set_level( int level );

/** 
 * @brief To get the logging level.
 * 
 * @return int , the logging level has been set previously.
 */
int gf_log_get_level();

/** 
 * @brief set the target of the logging info. It will override the original target list.
 * The new target list has only one target.
 * @param target,char*,the target to set,for example console,file:///tmp/disk1/log.log&max_size=5M
 */
int gf_log_set_targets( char* targets );

/** 
 * @brief Add a target to the target list.
 * 
 * @param target,char*,the target to add.for example console,file:///tmp/disk1/log.log&max_size=5M
 */
int gf_log_add_target( char* target );

/** 
 * @brief Remove a target from the target list.
 * 
 * @param target,char*, the target to del.
 */
int gf_log_del_target( char* target );

/** 
 * @brief Remove all targets from the target list.
 */
void gf_log_clear_alltarget();

/** 
 * @brief return the target list.
 *
 * @return the target list
 */
char* gf_log_get_targets();


/** 
 * @brief set some modules to output log,it will override thr original log modules
 * @param mods,char*,the modules to output log.for example ipanel,media,dhcp
 */
int gf_log_set_mods( char* mods );

/** 
 * @brief Add a log modules  to the log modules list.
 * 
 * @param mods,char*,the modules to output log.for example ipanel,media,dhcp
 */
int gf_log_add_mod( char* mod );

/** 
 * @brief Remove a log modules from the log modules list.
 * 
 * @param mods,char*, the modules to del.
 */
int gf_log_del_mod( char* mod );

/** 
 * @brief Remove all log modules from the log modules list.
 */
int gf_log_clear_allmods();

/** 
 * @brief set to output log of all moduls 
 */
int gf_log_add_allmods();

/** 
 * @brief return the log modules list.
 *
 * @return the log modules list
 */
char* gf_log_get_mods();

/**
 * @brief enable log modules
 */
int gf_log_enable_mod(char *mod, int enable);

/** 
 * @brief Output the logging info.
 * 
 * @param level int, the level of the logging info
 * @param mod  char*,the modduls name
 * @param file  char*,the file name
 * @param line  int,the line in the file
 * @param format char*, format string.
 * @param ... 
 * 
 * @return int , the logging level has been set previously.
 */
int gf_log( int level,const char* mod,const char* file,int line,const char *format, ... );


/**
 * @brief print current log output info
 */
void gf_log_print_state();

/**
 * @brief Output the log to ftpfile
 * @param buf char*, the log address
 * @param bufsize int, the log total length
 * @param target char*, the target log info
 */
typedef void (*ftp_output_callback)(char *buf, int bufsize, char *target); 

void gf_log_set_ftp_callback(ftp_output_callback callback);



#ifdef __cplusplus
}
#endif

#endif /* __GFLOG_H__ */


