/************************************************************
* AUTHOR: lijian / huanghuaming / chenkai
* CONTENT: 线程函数接口
* NOTE:	
* HISTORY:
* [2005-9-16] created
***********************************************************/
#include "swapi_linux.h"
#include <sys/times.h>
#include <sys/syscall.h>
#include "swmem.h"
#include "swthrd.h"
#include "swlog.h"
#include "swmutex.h"
#include "swtype.h"
#define MAX_THRD_NUM	64
#define swos_malloc malloc
#define swos_free free

typedef struct
{
	/* 线程ID */
	pthread_t m_tid;
	/* 线程属性对象 */
	pthread_attr_t m_attr;
	/* 父线程PID */
	pid_t m_ppid;
	/* 线程PID */
	pid_t m_pid;
	/*线程调度策略*/
	int m_policy;
	/* 线程的优先级 */
	int m_priority;
	/* 信号灯 */
	sem_t m_sem;
	/* 线程回调函数 */
	PThreadHandler m_pHandler;
	/* 回调函数参数 */
	unsigned long m_wParam;
	unsigned long m_lParam;	
	
	/* 是否暂停 */
	int bPause;
	/* 表示当前线程是否退出 */
	int m_bExit;
	/* 任务名称 */
	char name[32];
}SThrdInfo;

static SThrdInfo *m_all[MAX_THRD_NUM];
static int m_ref = -1;
static HANDLE m_mutex = NULL;
static int m_global_policy = -1;

static void* SemaThreadHandler( void* lpParam );

/* 打开线程，打开后处于暂停状态 */
HANDLE sw_thrd_open( char *name, unsigned char priority, int policy, int stack_size, 
					PThreadHandler pHandler, unsigned long wParam, unsigned long lParam )
{
	int i, rc;
	pthread_attr_t *pAttr = NULL;
	SThrdInfo *info = NULL;
	struct sched_param param;

	int max_priority;
	int min_priority;

	if( m_ref < 0 )
	{
		memset( m_all, 0, sizeof(m_all) );
		m_ref = 0;
		m_mutex = sw_mutex_create();


	}
	
	if(m_mutex)
		sw_mutex_lock(m_mutex);
	
	/* 检测自己退出的线程, 释放资源 */
	if(m_ref > MAX_THRD_NUM - 2)
	{
		
		for( i=0; i<MAX_THRD_NUM; i++ )
		{
			if(m_all[i] &&  m_all[i]->m_tid  != 0 && m_all[i]->m_pHandler == NULL)
			{
				printf("Release self-exit thread<%s>\n", m_all[i]->name);
				
				pthread_attr_destroy( &m_all[i]->m_attr );
				sem_destroy( &m_all[i]->m_sem );
				memset(m_all[i], 0, sizeof(SThrdInfo));
			
				swos_free(m_all[i]);
				m_all[i] = NULL;
			
				m_ref--;
			}
		}
	}
	
	if(m_ref >= MAX_THRD_NUM)
	{
		printf("Thread num reach max!!\n");
		sw_mutex_unlock(m_mutex);
		return NULL;
		
	}
	info = swos_malloc(sizeof(SThrdInfo));
	
	if( info == NULL )
	{
		printf("error: sw_thrd_open() failed\n" );
		if(m_mutex)
			sw_mutex_unlock(m_mutex);
	
		return NULL;
	}
	
	memset( info, 0, sizeof(SThrdInfo) );
	info->m_pHandler = pHandler;
	info->m_wParam = wParam;
	info->m_lParam = lParam;
	info->m_policy = policy;
	info->m_priority = (int)priority;
	info->bPause = 1;

	if(m_global_policy != -1)
	{
		info->m_policy = m_global_policy;
	}
	
	strncpy( info->name, name, sizeof( info->name )-1 );
	
	if(info->m_policy < SW_SCHED_NORMAL || info->m_policy > SW_SCHED_RR)
		info->m_policy = SW_SCHED_NORMAL;
	
	/* 初始化线程属性对象 */
	pAttr = &(info->m_attr);
	pthread_attr_init( pAttr );

	//设置线程堆栈大小
	if( stack_size < 65536 )
		stack_size = 65536;
	rc = pthread_attr_setstacksize(pAttr, stack_size);
	if(rc != 0)
	{
		unsigned int defsize = 0;
		pthread_attr_getstacksize(pAttr, &defsize);
		printf("sw_thrd_open [%s]: set stack size failed! %d, using default size %d\n", name, stack_size, defsize);
	}
	
	pthread_attr_setdetachstate(pAttr, PTHREAD_CREATE_DETACHED);
	/*设置调度策略*/
	pthread_attr_setschedpolicy( pAttr, info->m_policy /* SCHED_RR */ );

	/*在linux系统上, 线程优先级取值范围为1~99, 1最低, 99最高*/
	param.sched_priority = 1;
	max_priority = sched_get_priority_max(info->m_policy);
	min_priority = sched_get_priority_min(info->m_policy);

	priority = max_priority - (priority * (max_priority - min_priority)) / 255;
	param.sched_priority = priority;
	/* 创建信号灯 */
	if( sem_init( &(info->m_sem), 0, 0 ) == -1 )
	{
		if( pAttr )
			pthread_attr_destroy( pAttr );
		goto ERROR_EXIT;
	}
	/* 开始运行线程 */
	if( pthread_create( &(info->m_tid), pAttr, SemaThreadHandler, (void*)info ) != 0 )
	{
		if( pAttr )
			pthread_attr_destroy( pAttr );
		sem_destroy( &(info->m_sem) );
		goto ERROR_EXIT;
	}
	//线程运行时设置优先级
	pthread_setschedparam(info->m_tid, info->m_policy, &param );

	/* 保存线程句柄 */
	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if( m_all[i] == NULL)
		{
			m_all[i] = info;
			m_ref++;
			break;
			
		}
	}
	printf("Thread-<%s : %d : %x> opened\n", info->name,i,(void*)info );

	if(m_mutex)
		sw_mutex_unlock(m_mutex);
	return info;

ERROR_EXIT:
	info->m_pHandler = NULL;
	info->m_tid  = 0;
	if(m_mutex)
		sw_mutex_unlock(m_mutex);
	printf( "error: sw_thrd_open() failed !\n");
	return NULL;
}

/* 关闭线程 */
void sw_thrd_close( HANDLE hThrd, int ms )
{
	int i = 0;
	int pthread_kill_err;

	SThrdInfo *info = (SThrdInfo *)hThrd;
	if( info == NULL )
		return;

	info->m_bExit = 1;
	sem_post( &(info->m_sem) );

	printf("Thread-<%s : %d : %x> closing [%d]...\n", info->name,i,(void*)info, ms);

	/* 等待线程自己退出 */
	while(info->m_pHandler && ms > 0)
	{
		usleep(10*1000);
		ms -= 10;
	}

	if(m_mutex)
		sw_mutex_lock(m_mutex);

	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if( info == m_all[i])
			break;
	}

	if( MAX_THRD_NUM <= i )
	{
		if(m_mutex)
			sw_mutex_unlock(m_mutex);	
		return;
	}


	if(info->m_pHandler)
	{
		pthread_kill_err = pthread_kill(info->m_tid,SIGQUIT);
        if(pthread_kill_err == ESRCH)
            printf( "Thread<%s> is canneled or not exist!!!\n",info->name);
        else if(pthread_kill_err == EINVAL)
            printf("The signal is illegal value!!!\n");
        else
            printf("Thread<%s> is killed!!!\n",info->name);
#if 0				
		/* 强制退出 */
		if ( !pthread_cancel(info->m_tid) )
		{
			printf( "Thread<%s> is canneled!!!\n",  info->name);
		}
		else
		{
			status = pthread_join(info->m_tid, &result);
			if (status == 0)
			{
				if (result == PTHREAD_CANCELED)
				{
					printf("Thread canceled!!!\n");//如果线程是被取消的，回打印。
				}
				else
				{
					printf ("result:%d\n", (int)result);
		
				}

			}
		}
#endif		
	}
	

	pthread_attr_destroy( &info->m_attr );
	sem_destroy( &info->m_sem );
	memset( info, 0, sizeof(*info) );
	m_all[i] = NULL;
	swos_free(info);
	
	m_ref--;

	if(m_mutex)
		sw_mutex_unlock(m_mutex);
}

/* 是否打开 */
unsigned int sw_thrd_is_openned( HANDLE hThrd )
{
	SThrdInfo *info = (SThrdInfo *)hThrd;
	if( info == NULL )
		return false;
	return info->m_tid != 0;
}

/* 设置线程优先级  */
void sw_thrd_set_priority( HANDLE hThrd, unsigned char priority )
{
	struct sched_param param;
	SThrdInfo *info = (SThrdInfo *)hThrd;
	if( info == NULL )
		return ;

	info->m_priority = (int)priority;
	priority = sched_get_priority_max(info->m_policy) - (priority * (sched_get_priority_max(info->m_policy) - sched_get_priority_min(info->m_policy))) / 255;
	param.sched_priority = priority;
	pthread_setschedparam(info->m_tid, SCHED_RR, &param );
		

}

/** 
 * @brief 取得线程优先级,priority的范围为[0,255],0表示优先级最高,255最低
 * 
 * @param hThrd 线程句柄 
 * 
 * @return 当前线程的优先级
 */
unsigned char sw_thrd_get_priority( HANDLE hThrd )
{
	SThrdInfo *info = (SThrdInfo *)hThrd;
	return info->m_priority;
	
}


/* 暂停 */
void sw_thrd_pause( HANDLE hThrd )
{
	SThrdInfo *info = (SThrdInfo *)hThrd;
	if( info )
		info->bPause = 1;
}

bool sw_thrd_is_paused(HANDLE hThrd)
{
	
	SThrdInfo *info = (SThrdInfo *)hThrd;
	if( info )
		return info->bPause;
	return false;
	
}

/* 继续 */
void sw_thrd_resume( HANDLE hThrd )
{
	SThrdInfo *info = (SThrdInfo *)hThrd;
	if( info )
	{
		info->bPause = 0;
		sem_post( &(info->m_sem) );
	}
}

/* 延迟(ms) */
void sw_thrd_delay( int timeout )
{
	usleep( timeout*1000 );
}

/* 取得系统运行时间(ms) */
unsigned int sw_thrd_get_tick()
{
	/* NO 1. getimeofday */
/* 	static bool firsttimehere = TRUE; */
/* 	static struct timeval timeorigin; */

/* 	struct timeval now; */

/* 	gettimeofday(&now,NULL); */

/* 	if( firsttimehere ) */
/* 	{ */
/* 		timeorigin = now; */
/* 		firsttimehere=FALSE; */
/* 	} */

/* 	return (now.tv_sec-timeorigin.tv_sec)*1000 + (now.tv_usec-timeorigin.tv_usec)/1000; */
	
	/* NO 2. linux jiffies */
/* 	static bool firsttimehere = true; */
/* 	static uint64_t timeorigin; */
/* 	uint64_t now = sw_sys_get_tick(); */
/* 	if(firsttimehere) */
/* 	{ */
/* 		timeorigin = now; */
/* 		firsttimehere = false; */
/* 	} */

/* 	return now - timeorigin; */

/* NO 3. times sysconf(_SC_CLK_TCK); he number of clock ticks per second can be obtained using */

	struct tms tm;
	static uint32_t timeorigin;
	static bool firsttimehere = true;

	uint32_t now = times(&tm);
	if(firsttimehere)
	{
		timeorigin = now;
		firsttimehere = false;
	}

	return (now - timeorigin)*10;
	
	
}

static void* SemaThreadHandler( void* lpParam )
{

	SThrdInfo *info = (SThrdInfo *)lpParam;

	info->m_ppid = getppid();
	info->m_pid =getpid();
	/* pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL); //允许退出线程 */
//	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); //设置立即取消

	while( !info->m_bExit )
	{
		
		if( info->bPause )
			sem_wait( &(info->m_sem) );
		/* 增加测试点 */
		/* pthread_testcancel(); */

		if( info->m_pHandler == NULL || info->m_bExit )
			break;

		if( ! info->m_pHandler( info->m_wParam, info->m_lParam ) )
			break;
	}

	info->m_pHandler = NULL;
	printf( "Thread-<%s> exit self\n", info->name );

	return NULL;
}

void sw_thrd_set_global_policy(int policy)
{
	m_global_policy = policy;
	

}

/* 打印信息 */
void sw_thrd_print()
{
	int i;

	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if(m_all[i] && m_all[i]->m_pHandler )
		{
			printf("%-2d 0x%08x %-14s ppid=%d pid=%d proc=0x%-8x policy=%d priority=%d pause=%d \n", 
				   i, (int)(m_all[i]), m_all[i]->name,m_all[i]->m_ppid, m_all[i]->m_pid, (int)m_all[i]->m_pHandler, m_all[i]->m_policy, m_all[i]->m_priority, m_all[i]->bPause);
		}
	}
	printf( "\nref = %d\n", m_ref );
}

int sw_thrd_getinfo( char* buf, int size )
{
	char szBuf[255];
	int i, n, len;

	*buf = 0;
	len = 0;
	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if(m_all[i] && m_all[i]->m_pHandler )
		{
			n = sprintf( szBuf, "%-2d 0x%08x %-14s proc=0x%-8x policy=%d priority=%-3d pause=%d ppid=%d pid=%d\n", 
				   i, (int)(m_all[i]), m_all[i]->name, (int)m_all[i]->m_pHandler,
				   m_all[i]->m_policy, m_all[i]->m_priority, m_all[i]->bPause, m_all[i]->m_ppid, m_all[i]->m_pid);
			if( len + n < size )
			{
				strcpy( buf+len, szBuf );
				len += n;
			}
		}
	}
	n = sprintf( szBuf, "\nref = %d\n", m_ref );
	if( len + n < size )
		strcpy( buf+len, szBuf );
	return len;
}

HANDLE sw_thrd_find_byname( char* name )
{
	int i;
	for( i=0; i<MAX_THRD_NUM; i++ )
	{
		if(m_all[i] && m_all[i]->m_pHandler && !strcmp(name, m_all[i]->name) )
			return m_all[i];
	}
	return NULL;
}

