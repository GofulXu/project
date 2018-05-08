/*
 * @file swlog.c
 * @brief The file defines the interfaces to manipulate logging information.
 * @author 	qushihui
 * @version 	%I%, %G%
 * @history
 * 			2010-07-01 qushihui created
 *			2010-12-03 chenkai  optimized
 */

//#include "swapi.h"

#ifndef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/un.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

#include "swudp.h"
#include "swsignal.h"
#include "swlog.h"
#include "swthrd.h"
#include "swtype.h"
#define LOG_TARGETS_MAX		6       //目标数组最大容量
#define LOG_MODS_MAX		128		//最多允许输出日志的模块数目128
#define LOG_BUF_SIZE 		16384	//每条日志最大的大小,需要确保本buf大小小于ftp日志申请的空间大小
#define LOG_MODSINFO_LENGTH	 1024	//mods 列表信息长度
#define LOG_TARGETSINFO_LENGTH	 1024	//tagets 列表信息长度
#define LOG_LOCAL_SOCKET_FILE    "/tmp/.logsocket"
#define LOG_FTP_BUF_SIZE	131072

//把四个字符转化成一个uin32_t型数据
#ifdef WORDS_BIGENDIAN
#define SW_FOURCC( a, b, c, d ) \
        ( ((uint32_t)d) | ( ((uint32_t)c) << 8 ) \
        | ( ((uint32_t)b) << 16 ) | ( ((uint32_t)a) << 24 ) )
#else
#define SW_FOURCC( a, b, c, d )   (((uint32_t)a) | ( ((uint32_t)b) << 8 ) | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )
#endif

//日志输出目标的类型
typedef enum
{
	LOG_TARGET_NULL = 0x00,
	LOG_TARGET_CONSOLE,
	LOG_TARGET_FILE,
	LOG_TARGET_UDP,
    LOG_TARGET_LOCALSOCK,
	LOG_TARGET_SYSLOG,
	LOG_TARGET_FTP,
	LOG_TARGET_MAX
}target_type_t;

//日志文件的状态
typedef enum
{
	LOG_FP_OFF =0x00,
	LOG_FP_OPEN,
	LOG_FP_ON,
	LOG_FP_FULL,
	LOG_FP_CLOSE,
	LOG_FP_MAX
}log_fp_state_t;

//日志输出目标
typedef struct 
{
	target_type_t  type;
	char  target_org[256];				//目标路径
	union
	{
		struct 	log_file_t
		{
			FILE* fp1;					//日志文件句柄
			uint32_t fp1_size;			//日志文件当前数据大小
			log_fp_state_t fp1_state;	//日志文件状态
			int fp1_fullcount;			//日志文件处于文件满时的计数
			FILE* fp2;					//日志文件句柄	
			uint32_t fp2_size;			//日志文件当前数据大小	
			log_fp_state_t fp2_state;	//日志文件状态
			int fp2_fullcount;			//日志文件处于文件满时的计数
			uint32_t maxsize;			//日志文件最大大小
			int count;					//日志文件的计数,用于生成日志文件名
			char path[256];				//日志文件的路径
		}logfile;

		struct log_udp_t
		{
			uint32_t ip;
			uint16_t port;

		}logudp;

#ifndef WIN32
        struct log_localsock_t
        {
            int    sock;
            struct sockaddr_un sock_un;
        }loglocalsock;
#endif
		
		struct log_ftp_t
		{
			char 	 *logbuf1;	//申请128K+2K的buf作为ftp传输的日志
			int 	 logbufpos1;	//记录logbuf1日志的长度
			char 	 *logbuf2;	//申请128K+2K的buf作为ftp传输的日志,2个日志轮流切换,使用线程处理ftp的日志输出
			int 	 logbufpos2;	//记录logbuf2日志的长度
			char	 *buf;		//记录当前日志buf(重logbuf1,2中获取,如果当前部分)
			int		 bufsize;	//记录当前使用的buf的日志长度,当bufsize快达到128K时,如果下一buf不可用(传输中)清空buf,从头开始输出
		}logftp;
	}log_union;	
}target_t;

//打印等级，默认OFF
static int m_level = LOG_LEVEL_OFF; 
//日志输出目标数组
static target_t m_targets_list[LOG_TARGETS_MAX];
//日志输出目标信息
static char m_targets_info[LOG_TARGETSINFO_LENGTH];
//日志输出目标数目										
static int m_targets_num = 0;	
//模块开关标志
static bool m_allmods_is_allowed = false;
//允许输出日志的模块信息
static char m_mods_info[LOG_MODSINFO_LENGTH];
//允许输出日志的模块列表
static uint32_t m_mods_list[LOG_MODS_MAX];
//不允许输出日志的模块列表
static uint32_t m_disable_mods_list[LOG_MODS_MAX];
//允许输出日志的模块书目
static int m_mods_num = 0;
//日志输出等级信息
static const char* m_level_info[] = { "A", "D", "I", "W", "E", "F" }; 
//日志buf
static char  m_logbuf[4][LOG_BUF_SIZE];	
//日志buf索引
static int m_logbuf_index=0;
//udp日志输出socket
static int m_udp_skt = -1;

/* ftp日志输出线程 */
static HANDLE m_thrd = NULL;
/* ftp日志输出的信号量 */
static HANDLE m_signal = NULL;
/* 是否退出ftp日志传输 */
static bool m_b_exit_ftplog = false;

/**
 *@brief 把日志模块的名字转成数值
 */
static uint32_t calc_imod(const char* mod);
/**
 *@brief 查找mod是否存在
 */
static int find_imod(uint32_t imod);
/**
 *@brief 判断模块是否允许输出日志
 */
static bool log_mod_is_allowed(const char* mod);
/**
 *@brief 输出日志
 */
static int log_output(char* logbuf,int size);
/**
 *@brief 打开文件
 */
static FILE* log_open_file(char* path,int count);
/**
 *@brief 查找日志输出目标
 */
static int log_find_target(char* target);
/**
 *@brief 找一个空的日志输出目标，以便存储新的日志输出目标
 */
static int log_find_empty_target( );

/* ftp日志输出线程 */
static bool ftplog_proc(unsigned long wParam, unsigned long lParam);

static ftp_output_callback m_ftpcallback = NULL;

/** 
 * @brief Initialize the logging API, prepare for logging.
 * 
 * @param level int,  The original level to record the logging infomation. You can use sw_log_set_level() to change it.
 * @param target char*,  The target of all logging infomation.
 * @param mods char*,The modules which allow to ouput log.
 * 
 * @return int , The status of this operation.  0 on success, else on failure.
 */
int sw_log_init( int level, char* targets, char* mods )
{
    int i = 0;
	if( level < LOG_LEVEL_ALL || level > LOG_LEVEL_OFF )
	{
		printf("level error:0-6\n ");
		return -1;
	}

#ifndef WIN32
    for( i = 0; i < LOG_TARGETS_MAX; i++ )
    {
        m_targets_list[i].log_union.loglocalsock.sock = -1;
    }

#endif
	m_level = level;	
	sw_log_set_targets( targets);
	sw_log_set_mods( mods);
	
	return 0;
}

/** 
 * @brief Release some resources allocated by sw_log_init().
 */
void sw_log_exit()
{
	int i = 0;
	m_level = LOG_LEVEL_OFF ;
	sw_log_clear_alltarget();
	sw_log_clear_allmods();
	while ( i++ < 5 && m_thrd)
	{
		m_b_exit_ftplog = true;
		if ( m_signal )
			sw_signal_give( m_signal );
		sw_thrd_delay(10);
	}
	if ( m_thrd )
		sw_thrd_close( m_thrd, 100 );
	if ( m_signal )
		sw_signal_destroy( m_signal );
	return ;
}

/** 
 * @brief set the logging level
 * 
 * @param level int, the logging level to set.
 */
int sw_log_set_level( int level )
{
	if( level < LOG_LEVEL_ALL )
		m_level = LOG_LEVEL_ALL;
	else if( level > LOG_LEVEL_OFF )
		m_level = LOG_LEVEL_OFF;
	else
		m_level = level;
	return 0;
}


/** 
 * @brief To get the logging level.
 * 
 * @return int , the logging level has been set previously.
 */
int sw_log_get_level()
{
	return m_level;
}

/** 
 * @brief  set the target of the logging info. It will override the original target list.
 * The new target list has only one target.
 * @param target,char*,the target to set,for example console,file:///tmp/disk1/log.log&max_size=5M
 */
int sw_log_set_targets( char* targets )
{	
	char* p =NULL;
	char* q =NULL;
	char target[256];

	if(targets == NULL)
		return -1;

	sw_log_clear_alltarget();
	p = targets;
	while( *p != '\0' )
	{
		q = p;
		while( *q !=',' && *q !='\0')
			q++;			

		if( (q-p) <= (int)(sizeof(target)-1))
		{
			strncpy(target,p,q-p);
			target[q-p]='\0';
		}
		else
		{
			strncpy(target,p,sizeof(target)-1);
			target[sizeof(target)-1] = '\0';
		}

		sw_log_add_target(target);

		if( *q == '\0')
			break;
		else
			p = q+1;
	}
	return m_targets_num;
}

/** 
 * @brief Add a target to the target list.
 * 
 * @param target,char*,the target to add.for example console,file:///tmp/disk1/log.log&max_size=5M
 *		for:ftp://ftpuser:111111@192.168.0.8/00-11-09-EF-B5-AF when update log swlog modules will pack as
 *			ftp://ftpuser:111111@192.168.0.8/00-11-09-EF-B5-AF_20110517142934.log
 */
int sw_log_add_target( char* target )
{
	int i =0;
	char* p = NULL;
	char* q = NULL;

	if( target == NULL )
		return -1;

	if( log_find_target(target)>=0 )
		return 0;
	
	i = log_find_empty_target();
	if( i < 0 )
	{
		printf("target is full,add target failed...\n");
		return -2;
	}
	target_t *p_target = &(m_targets_list[i]);
	
    memset(p_target->target_org, 0, sizeof(p_target->target_org));
	strncpy(p_target->target_org,target,sizeof(p_target->target_org) - 1);
	if( strncmp(target,"console",strlen("console")) == 0 )
	{
		p_target->type = LOG_TARGET_CONSOLE;
		m_targets_num ++;
	}
#ifndef WIN32
    else if( strncmp( target, "localsock", strlen("localsock") ) == 0 )
    {
        if( p_target->log_union.loglocalsock.sock < 0 )
        {
            p_target->type = LOG_TARGET_LOCALSOCK;
            p_target->log_union.loglocalsock.sock = socket( AF_UNIX, SOCK_DGRAM, 0 );
            p_target->log_union.loglocalsock.sock_un.sun_family = AF_LOCAL;
            strcpy( p_target->log_union.loglocalsock.sock_un.sun_path, LOG_LOCAL_SOCKET_FILE );
            m_targets_num ++;
        }
    }
#endif
	else if( strncmp(target,"file://",strlen("file://")) == 0)
	{
		p = strstr(p_target->target_org,"&max_size=");
		if( p )
		{
			p_target->log_union.logfile.maxsize = strtol(p+strlen("&max_size="),&q,0);
			if(q)
			{
				if( *q == 'M' || *q=='m' )
					p_target->log_union.logfile.maxsize *= 1024*1024;
				else if( *q == 'K' || *q=='k' )
					p_target->log_union.logfile.maxsize *= 1024;
	
				//最小文件大小128K
				if( p_target->log_union.logfile.maxsize < 128*1024)
					p_target->log_union.logfile.maxsize = 128*1024;
			}
			*p = '\0';
            memset(p_target->log_union.logfile.path, 0, sizeof(p_target->log_union.logfile.path));
			strncpy(p_target->log_union.logfile.path,p_target->target_org+strlen("file://"),sizeof(p_target->log_union.logfile.path) - 1);
			*p = '&';
		}
		else
		{
			p_target->log_union.logfile.maxsize = 512 *1024;	//缺省文件大小为512K
            memset(p_target->log_union.logfile.path, 0, sizeof(p_target->log_union.logfile.path));
			strncpy(p_target->log_union.logfile.path,target+strlen("file://"),sizeof(p_target->log_union.logfile.path) - 1);
		}
		p_target->log_union.logfile.count = 0;
		p_target->log_union.logfile.fp1_state = LOG_FP_OPEN;
		p_target->log_union.logfile.fp1=log_open_file(p_target->log_union.logfile.path,p_target->log_union.logfile.count);
		if( p_target->log_union.logfile.fp1 )
		{
			p_target->log_union.logfile.count++;
			if( p_target->log_union.logfile.count >= 4 )
			{	
				p_target->log_union.logfile.count = 0;
			}
			p_target->log_union.logfile.fp1_size = 0;
			p_target->log_union.logfile.fp1_fullcount = 0;
			p_target->log_union.logfile.fp1_state = LOG_FP_ON;
		}
		else
		{
			printf("Open file %s \n failed ...\n",p_target->log_union.logfile.path);			
			p_target->log_union.logfile.fp1_state	= LOG_FP_OFF;
			return -3;
		}
		p_target->log_union.logfile.fp2 = NULL;
		p_target->log_union.logfile.fp2_size = 0;
		p_target->log_union.logfile.fp2_fullcount = 0;
		p_target->log_union.logfile.fp2_state	=LOG_FP_OFF;

		p_target->type = LOG_TARGET_FILE;
		m_targets_num ++;
	}
	else if( strncmp(target,"udp://",6) == 0 )
	{
		int j=0;
		unsigned short port = 0;
		unsigned int   ip = 0;
		p = target+6;
		q = strchr(p, ':');
		if( q )
		{
			*q = 0;
			q++;
			for( j=0; '0'<=q[j] && q[j]<='9'; j++ );
			port = atoi(q);
		}
		ip = inet_addr(p);
		if( 0<ip && ntohl(ip)<0xe0000000 && (q==NULL || (j>0 && q[j]==0)) )
		{
			p_target->log_union.logudp.ip = ip;
			p_target->log_union.logudp.port = htons(port==0 ? 514 : port);
			p_target->type = LOG_TARGET_UDP;
			m_targets_num ++;
		}
		else
		{
			return -4;
		}
		if(q)
			q[-1] = ':';
	}
	else if ( strncmp(target,"ftp://",6) == 0 )
	{/* 如果提前打开有可能打不开文件 */
		p_target->type = LOG_TARGET_FTP;
		p_target->log_union.logftp.logbuf1 = malloc( LOG_FTP_BUF_SIZE + 1);
		p_target->log_union.logftp.logbuf2 = malloc( LOG_FTP_BUF_SIZE + 1);
		if ( p_target->log_union.logftp.logbuf1 != NULL && p_target->log_union.logftp.logbuf2 != NULL )
		{
			if ( m_thrd == NULL )
				m_thrd = sw_thrd_open("tFTPLogProc", 100, SW_SCHED_RR, 16*1024, (PThreadHandler)ftplog_proc, 0, 0);
			if ( m_signal == NULL )
				m_signal = sw_signal_create(0, 1);
		}
		if ( p_target->log_union.logftp.logbuf1 == NULL || p_target->log_union.logftp.logbuf2 == NULL || m_thrd == NULL || m_signal == NULL )
		{
			p_target->type = LOG_TARGET_NULL;
			if ( p_target->log_union.logftp.logbuf1 != NULL )
			{
				free( p_target->log_union.logftp.logbuf1 );
				p_target->log_union.logftp.logbuf1 = NULL;
			}
			if ( p_target->log_union.logftp.logbuf2 != NULL )
			{
				free( p_target->log_union.logftp.logbuf2 );
				p_target->log_union.logftp.logbuf2 = NULL;
			}
			if ( m_thrd != NULL )
			{
				sw_thrd_close( m_thrd, 100);
				m_thrd = NULL;
			}
			if ( m_signal != NULL )
			{
				sw_signal_destroy( m_signal );
				m_signal = NULL;
			}
		}
		else
		{
			m_targets_num ++;
			p_target->log_union.logftp.buf = p_target->log_union.logftp.logbuf1;
			p_target->log_union.logftp.bufsize = 0;
			p_target->log_union.logftp.logbufpos1 = 0;			
			p_target->log_union.logftp.logbufpos2 = 0;
			sw_thrd_resume( m_thrd );
		}
	}
	//添加target
    m_targets_info[sizeof(m_targets_info) - 1] = '\0';
	if( m_targets_info[0] != '\0')
		strncat(m_targets_info,",",sizeof(m_targets_info) - 1);
	strncat(m_targets_info,target,sizeof(m_targets_info) - 1);
	return 0;
}

/** 
 * @brief Remove a target from the target list.
 * 
 * @param target,char*, the target to del.
 */
int sw_log_del_target( char* target )
{
	target_type_t  type;	
	target_t* p_target = NULL;
	int i = 0;
	char *p = NULL;
	char *q = NULL;

	if( target == NULL )
		return -1;

	i = log_find_target(target);

	if( i >= 0 )
		p_target = &(m_targets_list[i]);

	if( p_target )
	{
		type = p_target->type;
		p_target->type = LOG_TARGET_NULL;
		//让当前的日志输出完
		usleep(50*1000);
		//关闭日志文件
		if( type == LOG_TARGET_FILE )
		{
			if(p_target->log_union.logfile.fp1_state !=  LOG_FP_OFF)
			{
				p_target->log_union.logfile.fp1_state = LOG_FP_CLOSE;
				if(p_target->log_union.logfile.fp1)
					fclose(p_target->log_union.logfile.fp1);
				p_target->log_union.logfile.fp1 = NULL;
				p_target->log_union.logfile.fp1_fullcount = 0; 
				p_target->log_union.logfile.fp1_state = LOG_FP_OFF;
			}
			if( p_target->log_union.logfile.fp2_state != LOG_FP_OFF )
			{
				p_target->log_union.logfile.fp2_state=LOG_FP_CLOSE;
				if(p_target->log_union.logfile.fp2)
					fclose(p_target->log_union.logfile.fp2);
				p_target->log_union.logfile.fp2 = NULL;
				p_target->log_union.logfile.fp2_fullcount = 0; 
				p_target->log_union.logfile.fp2_state = LOG_FP_OFF;
			}
		}
#ifndef WIN32
        else if( type == LOG_TARGET_LOCALSOCK )
        {
            if( p_target->log_union.loglocalsock.sock >= 0 )
            {
                shutdown( p_target->log_union.loglocalsock.sock, SHUT_RDWR );
                close( p_target->log_union.loglocalsock.sock );
                p_target->log_union.loglocalsock.sock = -1;
            }
        }
#endif
		else if ( type == LOG_TARGET_FTP )
		{
			if ( p_target->log_union.logftp.logbuf1 != NULL )
				free( p_target->log_union.logftp.logbuf1 );
			p_target->log_union.logftp.logbuf1 = NULL;
			if ( p_target->log_union.logftp.logbuf2 != NULL )
				free( p_target->log_union.logftp.logbuf2 );
			p_target->log_union.logftp.logbuf2 = NULL;
			p_target->log_union.logftp.logbufpos1 = 0;
			p_target->log_union.logftp.logbufpos2 = 0;
			p_target->log_union.logftp.buf = NULL;
			m_b_exit_ftplog = true;
			sw_signal_give(m_signal);
			sw_thrd_delay(1);
		}
		memset(p_target->target_org,0,sizeof(p_target->target_org));
		m_targets_num --;

		//从targets信息里删除target
		p = strstr(m_targets_info,target);
		if( p )
		{
			q = p;
			while(*q !=',' && *q != '\0' )
				q++;
			if( *q == ',' )
				q++;
			memmove(p,q,strlen(q)+1);
			if( m_targets_info[0] != '\0' && *p == '\0')
			{
				memmove(p-1,p,strlen(p)+1);
			}
		}
	}
	return 0;
}

/** 
 * @brief Remove all targets from the target list.
 */
void sw_log_clear_alltarget()
{
	
	int level = m_level;
	int count = 0;
	int i;
	target_t* p_target = NULL;
	//设置log_level 
	m_level = LOG_LEVEL_OFF;

	//delay 50ms,让没有正在输出的日志输出完
	usleep(50*1000);

	memset(m_targets_info,0,sizeof(m_targets_info));
	//关闭打开的文件句柄
	for( i=0;i<LOG_TARGETS_MAX && count<m_targets_num;i++)
	{
		p_target = &(m_targets_list[i]);
		if( p_target->type == LOG_TARGET_FILE )
		{
			if(p_target->log_union.logfile.fp1_state !=  LOG_FP_OFF)
			{
				p_target->log_union.logfile.fp1_state = LOG_FP_CLOSE;
				if(p_target->log_union.logfile.fp1)
					fclose(p_target->log_union.logfile.fp1);
				p_target->log_union.logfile.fp1 = NULL;
				p_target->log_union.logfile.fp1_fullcount = 0; 
				p_target->log_union.logfile.fp1_state = LOG_FP_OFF;
			}
			if( p_target->log_union.logfile.fp2_state != LOG_FP_OFF )
			{
				p_target->log_union.logfile.fp2_state=LOG_FP_CLOSE;
				if(p_target->log_union.logfile.fp2)
					fclose(p_target->log_union.logfile.fp2);
				p_target->log_union.logfile.fp2 = NULL;
				p_target->log_union.logfile.fp2_fullcount = 0; 
				p_target->log_union.logfile.fp2_state = LOG_FP_OFF;
			}
		}
#ifndef WIN32
        else if( p_target->type == LOG_TARGET_LOCALSOCK )
        {
            if( p_target->log_union.loglocalsock.sock > 0 )
            {
                shutdown( p_target->log_union.loglocalsock.sock, SHUT_RDWR );
                close( p_target->log_union.loglocalsock.sock );
                p_target->log_union.loglocalsock.sock = -1;
            }
        }
#endif
		else if ( p_target->type == LOG_TARGET_FTP )
		{
			if ( p_target->log_union.logftp.logbuf1 != NULL )
				free( p_target->log_union.logftp.logbuf1 );
			p_target->log_union.logftp.logbuf1 = NULL;
			if ( p_target->log_union.logftp.logbuf2 != NULL )
				free( p_target->log_union.logftp.logbuf2 );
			p_target->log_union.logftp.logbuf2 = NULL;
			p_target->log_union.logftp.logbufpos1 = 0;
			p_target->log_union.logftp.logbufpos2 = 0;
			p_target->log_union.logftp.buf = NULL;
			m_b_exit_ftplog = true;
			sw_signal_give(m_signal);
		}
		if(p_target->type != LOG_TARGET_NULL) 
		{
			count++;
			p_target->type = LOG_TARGET_NULL;
			memset(p_target->target_org,0,sizeof(p_target->target_org));
		}
	}
	m_targets_num = 0;
	m_level = level;
	return ;
}

/** 
 * @brief return the target list.
 *
 * @return the target list
 */
char* sw_log_get_targets()
{
	return m_targets_info;
}


/** 
 * @brief set some modules to output log,it will override thr original log modules
 * @param mods,char*,the modules to output log.for example ipanel,media,dhcp
 */
int sw_log_set_mods( char* mods )
{
	char* p =NULL;
	char* q =NULL;
	char tmod[64];

	if( mods == NULL )
		return -1;

	m_mods_num = 0;
    memset(m_mods_info, 0, sizeof(m_mods_info));
	strncpy(m_mods_info,mods,sizeof(m_mods_info) - 1);	
	memset(m_mods_list,0,sizeof(m_mods_list));
	memset(m_disable_mods_list, 0, sizeof(m_disable_mods_list));
	p = m_mods_info;
	while( *p != '\0' )
	{
		q = p;
		while( *q !=',' && *q !='\0')
		{
			q++;			
		}
		if( (q-p) <= (int)sizeof(tmod)-1 )
		{
			strncpy(tmod,p,q-p);
			tmod[q-p]='\0';
		}
		else
		{
			strncpy(tmod,p,sizeof(tmod)-1);
			tmod[sizeof(tmod)-1] = '\0';
		}
		m_mods_list[m_mods_num] = calc_imod(tmod);	
		if( m_mods_list[m_mods_num] == SW_FOURCC('a','l','l',0) )
			m_allmods_is_allowed = true;

		m_mods_num++;

		if( *q == '\0')
			break;
		else
			p = q+1;
	}
	return m_mods_num;
}

/** 
 * @brief Add a log module to the log modules list.
 * 
 * @param mods,char*,the modules to output log.for example ipanel,media,dhcp
 */
int sw_log_add_mod( char* mod)
{
	uint32_t imod = 0;
	
	if( mod == NULL )
		return -1;

	imod = calc_imod(mod);

	if( imod == SW_FOURCC('a','l','l',0) )
		m_allmods_is_allowed = true;

	//查找模块是否存在，如果存在直接返回
	if( find_imod(imod)>=0 )
		return 0;
	//如果队列已经满,则增加失败
	if( m_mods_num == LOG_MODS_MAX )
		return -2;
	//增加mod
	if( m_mods_info[0] == '\0')
	{
        m_mods_info[sizeof(m_mods_info) - 1] = '\0';
		strncat(m_mods_info,mod,sizeof(m_mods_info) - 1);
	}
	else
	{
		sprintf(m_mods_info,"%s,",m_mods_info);
        m_mods_info[sizeof(m_mods_info) - 1] = '\0';
		strncat(m_mods_info,mod,sizeof(m_mods_info) - 1);
	}
	m_mods_list[m_mods_num] = imod;
	m_mods_num ++;
	return 0;
}

/** 
 * @brief Remove a log module from the log modules list.
 * 
 * @param mods,char*, the modules to del.
 */
int sw_log_del_mod( char* mod )
{
	int i =0;
	char* p =  NULL;
	char* q = NULL;
	uint32_t imod =0;
	
	if( mod == NULL )	
		return -1;

	imod = calc_imod( mod );
	
	if( imod == SW_FOURCC('a','l','l',0) )
		m_allmods_is_allowed = false;

	if( (i=find_imod(imod)) <0 )
		return 0;

	m_mods_list[i] = 0;
	m_mods_num--;

	//从mod信息里删除mod
	p = strstr(m_mods_info,mod);
	if( p )
	{
		q = p;
		while(*q !=',' && *q != '\0' )
			q++;
		if( *q == ',' )
			q++;
		memmove(p,q,strlen(q)+1);

		if( m_targets_info[0] != '\0' && *p == '\0')
		{
			memmove(p-1,p,strlen(p)+1);
		}
	}

	return 0;
}

/** 
 * @brief Remove all log modules from the log modules list.
 */
int sw_log_clear_allmods()
{	
	memset(m_mods_info,0,sizeof(m_mods_info));
	m_mods_num = 0;
	memset(m_mods_list,0,sizeof(m_mods_list));
	m_allmods_is_allowed = false;
	return 0;
}
static int log_mod_is_disable(const char *mod)
{
	uint32_t imod = calc_imod( mod );
	int i = 0; 
	for ( i = 0; i < sizeof(m_disable_mods_list)/sizeof(m_disable_mods_list[0]); i++)
	{
		if ( m_disable_mods_list[i] == imod && m_disable_mods_list[i] != 0)
			return 1;
	}
	return 0;
}
/**
 * @brief disable log modules
 */
int sw_log_enable_mod(char *mod, int enable)
{
	uint32_t imod = calc_imod( mod );
	int i = 0;
	if ( enable )
	{
		for ( i = 0 ; i < sizeof(m_disable_mods_list)/sizeof(m_disable_mods_list[0]); i++)
		{
			if ( m_disable_mods_list[i] == imod )
			{
				m_disable_mods_list[i] = 0;
				return 0;
			}
		}
	}
	else
	{
		for ( i = 0 ; i < sizeof(m_disable_mods_list)/sizeof(m_disable_mods_list[0]); i++)
		{
			if ( m_disable_mods_list[i] == imod )
			{
				return 0;
			}
		}
		for ( i = 0 ; i < sizeof(m_disable_mods_list)/sizeof(m_disable_mods_list[0]); i++)
		{
			if ( m_disable_mods_list[i] == 0 )
			{
				m_disable_mods_list[i] = imod;
				return 0;
			}
		}			
	}
	return 0;
}

/** 
 * @brief set to output log of all moduls 
 */
int sw_log_add_allmods()
{
	m_allmods_is_allowed = true;
	return 0;
}


/** 
 * @brief return the log modules list.
 *
 * @return the log modules list
 */
char* sw_log_get_mods()
{
	return m_mods_info;
}

/**
 * @brief print current log output info
 */
void sw_log_print_state()
{
	printf("Log state: \n");
	printf("	level: %d\n",m_level);
	printf("	target[%d]:%s\n",m_targets_num,m_targets_info);
	printf("	allow all mods:%d\n",m_allmods_is_allowed);
	printf("	mods[%d]:%s\n",m_mods_num,m_mods_info);
}

/** 
 * @brief output the logging info.
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
int sw_log( int level,const char* mod, const char* file, int line, const char *format, ... )
{
	char* logbuf = NULL;
	int index;
	int size;
	va_list args;
    
	//没有打印目标时返回
	if( 0 == m_targets_num )
		return  -1;

	if(m_level >= LOG_LEVEL_OFF )		
		return -2;
	
	//判断打印等级
	if( level < m_level || level < 0 || level > LOG_LEVEL_FATAL)
		return -3;
	if (log_mod_is_disable(mod))
		return -5;
	//判断模块是否允许输出日志
	if( !log_mod_is_allowed(mod) )
		return -4;

	//选择生成日志的buf
	index = m_logbuf_index + 1;
	if( index >= (int)(sizeof(m_logbuf) / sizeof(m_logbuf[0])) )
	{
		index=0;
	}
	m_logbuf_index = index; 
	logbuf=m_logbuf[index];

	//填充日志头信息
	size = snprintf(logbuf,LOG_BUF_SIZE,"[%u]%s[%s %s %d] ", sw_thrd_get_tick(), m_level_info[level],mod,file,line);

	//填充具体日志信息
	va_start( args, format );
	size +=vsnprintf(logbuf+size,LOG_BUF_SIZE-1-size,format, args );
	va_end( args );

	//置字符串结束符
	if( size >= LOG_BUF_SIZE )
		size = LOG_BUF_SIZE - 1;
	logbuf[size] = 0;
	return log_output(logbuf,size);
}

/**
 *@brief设置ftp回调函数
*/
void sw_log_set_ftp_callback(ftp_output_callback callback)
{
	m_ftpcallback = callback;
}

/**
 *@brief 把日志模块的名字转成数值
 */
static uint32_t calc_imod(const char* mod)
{
	unsigned char t[5];
	memset(t,0,sizeof(t));
	strncpy((char*)t,mod,sizeof(t) - 1);
	return SW_FOURCC(t[0],t[1],t[2],t[3]);
}
/**
 *@brief 查找是否有相同的模块
 */
static int find_imod(uint32_t imod)
{
	int i=0;
	int count = 0;
	for( i =0;i<LOG_MODS_MAX && count<m_mods_num;i++)
	{
		if( m_mods_list[i] != 0 )
			count ++;
		if( m_mods_list[i] == imod)
			return i;
	}
	return -1;
}

/**
 *@brief 判断模块是否允许输出日志
 */
static bool log_mod_is_allowed(const char* mod)
{
	if( m_allmods_is_allowed )
		return true;

	return (find_imod(calc_imod(mod)) >= 0);
}

/**
 *@brief 打开文件
 */
static FILE* log_open_file(char* path,int count)
{	
	char fname[256];
	char suffix[32];
	char index[8];
	char* p = NULL;
	FILE* fp = NULL;

	memset(suffix,0,sizeof(suffix));
    memset(fname, 0, sizeof(fname));
	strncpy(fname,path,sizeof(fname) - 1);
	p = strchr(fname,'.');
	if(p)
	{
		strncpy(suffix,p,sizeof(suffix) - 1);
		*p = '\0';
	}
	snprintf(index,sizeof(index),"%d",count);
    fname[sizeof(fname) - 1] = '\0';
	strncat(fname,index,sizeof(fname) - 1);
	strncat(fname,suffix,sizeof(fname) - 1);
	
	fp = fopen(fname,"w");
	printf("goeful-->open_file:%s,fp:%d\n",fname, fp);
	//删除旧的日志文件
	if( count >= 2 )
	{	
        memset(fname, 0, sizeof(fname));
		strncpy(fname,path,sizeof(fname) - 1);
		p = strchr(fname,'.');
		if( p )
			*p = '\0';
		snprintf(index,sizeof(index),"%d",count-2);
        fname[sizeof(fname) - 1] = '\0';
		strncat(fname,index,sizeof(fname) - 1);
		strncat(fname,suffix,sizeof(fname) - 1);
		remove(fname);
	}
	return fp;
}

/**
 *@brief 查找日志输出目标
 */
static int log_find_target(char* target)
{
	int i =0;
	for( i =0;i<LOG_TARGETS_MAX;i++)
	{
		if( m_targets_list[i].type != LOG_TARGET_NULL
			&&  strcmp(target,m_targets_list[i].target_org)==0 )
			return i;
	}
	return -1;

}

/**
 *@brief 找一个空的日志输出目标，以便存储新的日志输出目标
 */
static int log_find_empty_target( )
{
	int i =0;
	for( i =0;i<LOG_TARGETS_MAX;i++)
	{
		if( m_targets_list[i].type == LOG_TARGET_NULL )
			return i;
	}
	return -1;
}


/**
 *@brief 输出日志
 */
static int log_output(char* logbuf,int size)
{
	int i=0;
	target_t* p_target = NULL;
	FILE* fp = NULL;
	int count  = 0;

	for( i = 0; i<LOG_TARGETS_MAX && count<m_targets_num ; i++ )
	{
		p_target = &(m_targets_list[i]);
		
		if( p_target->type != LOG_TARGET_NULL)
			count++;

		//日志直接输出到控制台
		if( p_target->type == LOG_TARGET_CONSOLE )
		{
			//printf( "%s", logbuf );
#ifdef ANDROID
#define LOG_TAG  "SWMEDIA"
#include <utils/Log.h>
            LOGE("%s", logbuf);
#else
			fprintf( stdout, "%s", logbuf );
#endif
		}
#ifndef WIN32
        else if( p_target->type == LOG_TARGET_LOCALSOCK )
        {
            if( p_target->log_union.loglocalsock.sock >= 0 )
            {
                sendto( p_target->log_union.loglocalsock.sock, 
                        logbuf, size + 1, 0, 
                        (struct sockaddr*)&(p_target->log_union.loglocalsock.sock_un), 
                        sizeof( p_target->log_union.loglocalsock.sock_un ) );
            }
        }
#endif
		//日志以文件的形式保存于内存或外挂设备
		else if( p_target->type == LOG_TARGET_FILE)
		{
			fp = NULL;
			//确定日志输出文件
			if( p_target->log_union.logfile.fp1_state == LOG_FP_FULL )
			{
				fp = p_target->log_union.logfile.fp1;
			}
			if(  p_target->log_union.logfile.fp2_state == LOG_FP_FULL )
			{
				fp = p_target->log_union.logfile.fp2;
			}

			if( p_target->log_union.logfile.fp1_state == LOG_FP_ON )
			{	
				fp = p_target->log_union.logfile.fp1;
				if( p_target->log_union.logfile.fp1_size >=(p_target->log_union.logfile.maxsize-8192))
				{
					p_target->log_union.logfile.fp1_fullcount = 0;
					p_target->log_union.logfile.fp1_state  = LOG_FP_FULL;
				}
			}
			if( p_target->log_union.logfile.fp2_state == LOG_FP_ON )
			{
				fp = p_target->log_union.logfile.fp2;
				if( p_target->log_union.logfile.fp2_size >=(p_target->log_union.logfile.maxsize-8192) )
				{
					p_target->log_union.logfile.fp2_state = LOG_FP_FULL;
					p_target->log_union.logfile.fp2_fullcount = 0;
				}
			}

			//处理LOG文件打开和关闭
			if( p_target->log_union.logfile.fp1_state == LOG_FP_FULL)
			{
				if( p_target->log_union.logfile.fp2_state == LOG_FP_OFF)
				{	
					p_target->log_union.logfile.fp2_state = LOG_FP_OPEN;
					p_target->log_union.logfile.fp2 = log_open_file(p_target->log_union.logfile.path,p_target->log_union.logfile.count);
					if( p_target->log_union.logfile.fp2 )
					{
						p_target->log_union.logfile.count++;
						if( p_target->log_union.logfile.count >= 4 )
						{	
							p_target->log_union.logfile.count = 0;
						}
						p_target->log_union.logfile.fp2_size = 0;
						p_target->log_union.logfile.fp2_state = LOG_FP_ON;
						fp = p_target->log_union.logfile.fp2;
					}
					else
					{	
						fprintf(stdout, "Open file %s \n failed ...\n",p_target->log_union.logfile.path);		
						p_target->log_union.logfile.fp2_state = LOG_FP_OFF;
					}
				}
				
				p_target->log_union.logfile.fp1_fullcount++;
				if( p_target->log_union.logfile.fp1_fullcount >= 8 
					&& p_target->log_union.logfile.fp1_state==LOG_FP_FULL )
				{
					p_target->log_union.logfile.fp1_state=LOG_FP_CLOSE;	
					if(p_target->log_union.logfile.fp1) 
						fclose(p_target->log_union.logfile.fp1);
					p_target->log_union.logfile.fp1 = NULL;
					p_target->log_union.logfile.fp1_fullcount = 0; 
					p_target->log_union.logfile.fp1_state = LOG_FP_OFF;
				}
			}
			if( p_target->log_union.logfile.fp2_state==LOG_FP_FULL)
			{	
				if( p_target->log_union.logfile.fp1_state == LOG_FP_OFF)
				{
					p_target->log_union.logfile.fp1_state = LOG_FP_OPEN;
					p_target->log_union.logfile.fp1 = log_open_file(p_target->log_union.logfile.path,p_target->log_union.logfile.count);	
					if( p_target->log_union.logfile.fp1)
					{
						p_target->log_union.logfile.fp1_size = 0;
						p_target->log_union.logfile.fp1_state = LOG_FP_ON;
						fp = p_target->log_union.logfile.fp1;
					}
					else
					{
						fprintf(stdout, "Open file %s \n failed ...\n",p_target->log_union.logfile.path);		
						p_target->log_union.logfile.fp1_state = LOG_FP_OFF;
					}
				}
				p_target->log_union.logfile.fp2_fullcount ++;
				if( p_target->log_union.logfile.fp2_fullcount >= 8
					&& p_target->log_union.logfile.fp2_state==LOG_FP_FULL )
				{
					p_target->log_union.logfile.fp2_state=LOG_FP_CLOSE;
					if(p_target->log_union.logfile.fp2)
						fclose(p_target->log_union.logfile.fp2);
					p_target->log_union.logfile.fp2 = NULL;
					p_target->log_union.logfile.fp2_fullcount = 0; 
					p_target->log_union.logfile.fp2_state = LOG_FP_OFF;
				}
			}
				
			//输出日志,到文件
			if(fp)
			{
				fputs( logbuf, fp);
				if( fp  == p_target->log_union.logfile.fp1 )
					p_target->log_union.logfile.fp1_size += size;
				else if( fp == p_target->log_union.logfile.fp2 )
					p_target->log_union.logfile.fp2_size += size;
			}
		}
		else if( p_target->type == LOG_TARGET_UDP )
		{
			if( m_udp_skt<0 )
				m_udp_skt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

			if( m_udp_skt>0 )
			{
				struct sockaddr_in sn;
				unsigned int slen = sizeof(sn);

				memset( &sn, 0, sizeof(sn) );
				slen = sizeof(sn);
				sn.sin_family = AF_INET;
				sn.sin_addr.s_addr = p_target->log_union.logudp.ip;
				sn.sin_port=  p_target->log_union.logudp.port;
				sendto( m_udp_skt, logbuf, size, 0, (struct sockaddr *)&sn, slen );	
			}
		}
		else if ( p_target->type == LOG_TARGET_FTP )
		{
			if ( p_target->log_union.logftp.bufsize + size >= LOG_FTP_BUF_SIZE )
			{/**/
				if ( p_target->log_union.logftp.buf == p_target->log_union.logftp.logbuf1 && p_target->log_union.logftp.logbufpos1 == 0)
				{/* logbufpos1非0 表示还没发送完全,有可能在发送中 */
					p_target->log_union.logftp.logbufpos1 = p_target->log_union.logftp.bufsize;
					p_target->log_union.logftp.buf = p_target->log_union.logftp.logbuf2;
					p_target->log_union.logftp.bufsize = 0;
					sw_signal_give( m_signal );
				}
				else if ( p_target->log_union.logftp.buf == p_target->log_union.logftp.logbuf2 && p_target->log_union.logftp.logbufpos2 == 0 )
				{
					p_target->log_union.logftp.logbufpos2 = p_target->log_union.logftp.bufsize;
					p_target->log_union.logftp.buf = p_target->log_union.logftp.logbuf1;
					p_target->log_union.logftp.bufsize = 0;
					sw_signal_give( m_signal );
				}
				else
				{
					p_target->log_union.logftp.bufsize = 0;
				}
			}
			if ( p_target->log_union.logftp.buf != NULL )
			{
				memcpy(&(p_target->log_union.logftp.buf[p_target->log_union.logftp.bufsize]), logbuf, size);
				p_target->log_union.logftp.bufsize += size;
				p_target->log_union.logftp.buf[p_target->log_union.logftp.bufsize] = '\0';
			}
		}
	}
	return size;
}

static bool ftplog_proc(unsigned long wParam, unsigned long lParam)
{
	sw_signal_wait( m_signal, -1 );
	if ( m_b_exit_ftplog )

	{
		m_thrd = NULL;
		return false;
	}
	//找到ftp日志的句柄
	int i = 0, count = 0;
	
	target_t* p_target = NULL;
	for( i = 0; i<LOG_TARGETS_MAX && count<m_targets_num ; i++ )
	{
		p_target = &(m_targets_list[i]);
		count++;
		if ( p_target->type == LOG_TARGET_FTP )
		{
			char *buf = NULL;
			int index = 0;
			int bufsize = 0;
			if ( p_target->log_union.logftp.logbufpos1 > 0 )
			{
				buf = p_target->log_union.logftp.logbuf1;
				index = 1;
				bufsize = p_target->log_union.logftp.logbufpos1;
			}
			else if ( p_target->log_union.logftp.logbufpos2 > 0 )
			{
				buf = p_target->log_union.logftp.logbuf2;
				index = 2;
				bufsize = p_target->log_union.logftp.logbufpos2;
			}
			if ( buf == NULL )
				continue;
			if ( m_ftpcallback != NULL )
			{
				m_ftpcallback( buf, bufsize, p_target->target_org);
			}
			if ( index == 1 )
			{
				p_target->log_union.logftp.logbufpos1 = 0;
			}
			else if ( index == 2 )
			{
				p_target->log_union.logftp.logbufpos2 = 0;
			}
		}
	}
	return true;
}
