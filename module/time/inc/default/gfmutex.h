/** 
 * @file gfmutex.h
 * @brief 互斥量函数接口
 * @author lijian / huanghuaming / chenkai
 * @date 2005-09-16
 */

#ifndef __GFMUTEX_H__
#define __GFMUTEX_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 * @brief 创建互斥量
 * 
 * @return 成功,返回互斥量句柄; 否则,返回空值
 */
HANDLE gf_mutex_create();

/** 
 * @brief 销毁互斥量
 * 
 * @param mutex 互斥量句柄
 */
void gf_mutex_destroy( HANDLE mutex );

/** 
 * @brief 加锁
 * 
 * @param mutex 互斥量句柄
 */
void gf_mutex_lock( HANDLE mutex );

/** 
 * @brief 解锁
 * 
 * @param mutex 互斥量句柄
 */
void gf_mutex_unlock( HANDLE mutex );

#ifdef __cplusplus
}
#endif

#endif  /* __GFMUTEX_H__  */
