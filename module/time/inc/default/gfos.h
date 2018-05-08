/**                                                              
 * @file gfos.h
 * @brief gfos 模块初始化
 * @author qushihui
 * @date 2010-07-09
 */

#ifndef __GFOS_H__
#define __GFOS_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief 添加配置库
 * 
 */
void gf_os_init();


/**
 * @brief 释放资源
 */
void gf_os_exit();

#ifdef __cplusplus
}
#endif

#endif //__GFOS_H__

