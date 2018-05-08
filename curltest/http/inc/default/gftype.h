/** 
 * @file gftype.h
 * @brief 类型定义
 * @author  Dou Hongchen 
 * @date 2007-11-19
 *
 */

#ifndef __GFTYPE_H__
#define __GFTYPE_H__

#ifndef uint8_t
#define uint8_t unsigned char
#endif
#ifndef uint16_t
#define uint16_t unsigned short
#endif
#ifndef uint32_t
#define uint32_t unsigned int 
#endif
#ifndef uint64_t
#define uint64_t unsigned long 
#endif

#ifndef size_t
#define size_t unsigned long 
#endif

#ifndef __cplusplus
#ifndef bool
#define bool uint8_t
#endif	
#ifndef true
#define true 1
#endif	

#ifndef false
#define false 0
#endif  

#endif


#ifndef HANDLE
#define HANDLE void*
#endif

#ifndef HANLDE
#define HANLDE HANDLE
#endif


#ifndef SYSHANDLE
#define SYSHANDLE HANDLE
#endif

#ifndef LPVOID
#define LPVOID void*
#endif

#ifndef BOOL
#define BOOL int
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


#ifdef GF_OK
#undef GF_OK
#endif

#ifdef GF_ERROR
#undef GF_ERROR
#endif

#ifdef GF_STATUS
#undef GF_STATUS
#endif 

typedef unsigned int					GF_STATUS;  	/**<Sunniwell函数返回类型*/
#define GF_OK							0x00000000		/**<返回成功*/
#define GF_ERROR 						0xFFFFFFFF 		/**<一般错误*/									
#define GF_ERROR_INVALID_PARAMETER 		0x80000000 		/**<无效的参数*/
#define GF_ERROR_NOMEMORY		 		0x80000001 		/**<内存不足*/
#define GF_ERROR_SYSERR			 		0x80000002 		/**<系统错误*/
#define GF_ERROR_NOTINIT				0x80000003 		/**<没有初始化*/
#define GF_ERROR_NOTSUPPORT				0x80000004 		/**<不支持*/
#define GF_ERROR_TIMEOUT				0x80000005 		/**<超时*/

#ifndef NO_WAIT
#define NO_WAIT       0
#endif

#ifndef INFINITE
#define INFINITE -1
#endif

#ifndef WAIT_FOREVER
#define WAIT_FOREVER INFINITE
#endif



#ifndef SOCKET
#define SOCKET int
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif


#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

/**
 * @点坐标定义
 */
typedef struct _point{
	
	int x;	/**<X坐标*/
	int y;	/**<Y坐标*/
}gfpoint_t;

/**
 * @矩形框定义
 */
typedef struct _rect{
	
	int x;				/**<X坐标*/
	int y;				/**<Y坐标*/
	unsigned int width; /**<宽度*/
	unsigned int height;/**<高度*/ 
}gfrect_t;

#endif /*__GFTYPR_H__*/
