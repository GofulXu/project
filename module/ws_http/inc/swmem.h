/** 
 * @file swmem.h
 * @brief 可分块/可跟踪内存泄漏位置的内存管理方式
 * @author hujinshui / huanghuaming
 * @date 2005-09-14
 */

#ifndef __SWMEM_H__
#define __SWMEM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "swtype.h"
/** 
 * @brief 对已分配的内存进行初始化 
 * 
 * @param buf 	已分配的内存地址
 * @param size 	已分配内存的大小 
 * @param align 字节分配的方式
 * 
 * @return 成功,返回内存句柄; 失败,返回空值
 */
HANDLE sw_mem_init( char *buf, int size, int align );

/** 
 * @brief 释放内存句柄
 * 
 * @param h 内存句柄
 */
void sw_mem_exit( HANDLE h );

/** 
 * @brief 内存清空
 * 
 * @param h 内存句柄
 */
void sw_mem_reset( HANDLE h );

/** 
 * @brief 检查是否有未释放的内存 
 * 
 * @param h 内存句柄
 */
void sw_mem_check( HANDLE h );
/** 
 * @brief 获取总的内存大小
 * 
 * @param h 内存句柄
 * 
 * @return 总的内存大小
 */
int sw_mem_get_total_size( HANDLE h );
/** 
 * @brief 取历史上最大已分配尺寸
 * 
 * @param h 内存句柄   
 * 
 * @return 最大已分配尺寸的大小
 */
int sw_mem_get_max_size( HANDLE h );
/** 
 * @brief 取现在最大分配尺寸
 * 
 * @param h 内存句柄
 * 
 * @return 现在分配尺寸的大小
 */
int sw_mem_get_cur_size( HANDLE h );
/** 
 * @brief 是否没有分配已分配的节点
 * 
 * @param h 内存句柄
 * 
 * @return 如果有已分配的节点,则返回真(true);否则,返回假(false)
 */
bool sw_mem_is_empty( HANDLE h );

/** 
 * @brief 从内存句柄所指向的内存中分配一段内存
 * 
 * @param h 内存句柄
 * @param size 分配内存的大小
 * @param filename 所在的当前文件名 
 * @param line 所在的当前行号
 * 
 * @return 成功返回分配后内存的地址; 否则,返回空值
 */
void *sw_mem_alloc( HANDLE h, int size, const char *filename, int line );
/** 
 * @brief 			释放内存句柄所指向的一段内存
 * 
 * @param h 		内存句柄
 * @param p 		指向的内存地址
 * @param filename 	所在的当前文件名
 * @param line 		所在的当前行号
 */
void sw_mem_free( HANDLE h, void *p, const char *filename, int line );
/** 
 * @brief 从内存句柄指向的内存中复制字符串
 * 
 * @param h 内存句柄
 * @param s 指向的字符串
 * @param filename 所在的当前文件名
 * @param line 所在的当前行号
 * 
 * @return 成功,返回复制后的字符串指针; 否则,返回空值
 */
char *sw_mem_strdup( HANDLE h, const char *s, const char *filename, int line );
/** 
 * @brief 在原有内存的基础上重新申请内存
 * 
 * @param h 内存句柄
 * @param p 指向原有的内存
 * @param size 分配内存的大小
 * @param filename 所在的当前文件名
 * @param line 所在的当前行号
 * 
 * @return 成功,返回实际分配后的新地址; 否则,返回空值
 */
void *sw_mem_realloc( HANDLE h, void *p, int size, const char *filename, int line );

extern HANDLE g_hSwHeap;
	
/** 
 * @brief 初试化内存管理全局堆
 * 
 * @param buf 指向的内存
 * @param size 指向内存的大小
 * @param align 内存管理的方式
 * 
 * @return 成功返回真(1); 否则,返回假(0)
 */
bool sw_heap_init(char* buf, int size, int align);
	
/** 
 * @brief 全局堆释放 
 */
void sw_heap_exit();
/** 
 * @brief 取得总的内存大小
 * 
 * @return 成功,返回总的内存大小; 否则,返回-1
 */
int sw_heap_get_total_size();
/** 
 * @brief 取现在最大分配尺寸
 * 
 * @return 成功,返回现在分配尺寸的大小; 否则,返回-1
 */
int sw_heap_get_cur_size();

#define sw_malloc(size) malloc(size)

#define sw_free(p)  free(p)

#define sw_strdup(s)  strdup(s)

#define sw_realloc(p, size) realloc(p, size)


#ifdef __cplusplus
}
#endif

#endif /*__SWMEM_H__*/

