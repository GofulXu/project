/* 
 * CONTENT: 定义NTP client(RFC 868/958/1305/2030 Network Time Protocol)
 * HISTORY:
 *		[2004-04-07] lijian created.
 *		[2005-10-13] chenkai modifyed the function name.
 *      [2006-12-04] douhongchen add linux ntp support.
 *      [2007-04-18] lizhibin modify function "ntp_client_proc".
 */

#include "swapi.h"
#include "swtxtparser.h"
#include "swntpclient.h"
#include "swsignal.h"
#include "swthrd.h"
#include "swparameter.h"
#include "swnetwork.h"
#include "swmodules_priv.h"
#include "swdns.h"
#include "timezone.h"
#include "swsystem.h"
//网络时间同步线程
static HANDLE m_h_thrd = NULL;
//重新获得网络时间的信号量
static HANDLE m_h_signal = NULL;    
//设置得到网络时钟时间间隔
static uint32_t m_n_interval = 4*3600*1000;

//网络时钟服务器(域名或者ip)
static char m_ntpsvr[128];
//系统当前时区
static int m_n_time_zone = 8*60;	//时区单位用分钟表示

static bool m_b_exit = false;
static bool m_b_give_signal = false;
static int m_ntperrorCode = 0;

//NTP：网络时间协议处理任务
static bool ntp_client_proc( uint32_t wparam, uint32_t lparam );
//从网络时钟服务器获取网络时钟
/* static int GetRFC868Time( unsigned long ntpsvr, time_t* RFC868Time ); */

/* 从网络时钟服务器获取时间。 如果tv!=NULL，就填充tv */
static  bool ntp_time_get(char* hostname, int timeout, struct timeval *tv);

static void set_timezone_env( int tz_minuteswest );

//连接错误是否已提示
static bool m_b_error_reported = false;
//连接是否成功过
static bool m_b_success_once = false;
//当前连接状态, true=连接成功, false=连接失败
static bool m_b_current_status = false;

static event_post_func   m_event_post   = NULL;
static HANDLE    m_event_handler         = NULL;

static uint32_t m_new_time = 0;

#ifdef SUPPORT_TTNET
static bool sw_turkey_summer_time();
static void sw_turkey_time_correction();
#endif
static unsigned long int sec_since_1970(unsigned long int  sec_since_1900) __attribute__((__unused__));
static unsigned long int time_zone_modify(unsigned long int seconds, int time_zone) __attribute__((__unused__));

/*summer time信息的结构*/
typedef struct sumtime_info
{
	int 	 time_zone; //夏令时区,其同步时间是分时间段判断
  	uint32_t start_time;    //当年夏令时UTC开始时间
  	uint32_t end_time;      //当年夏令时UTC结束时间
	int  	 flag;          //夏令时标志0,不启动夏令时,1启动夏令时
	bool	 b_sumtime;		//当前是夏令时
}sumtime_info_t;
static	sumtime_info_t sumtime;
static HANDLE m_sumt_thread = NULL;
static bool m_imm_ntp = false;
void sw_immediately_ntp( int timeminute)
{
	m_imm_ntp = true;
	m_n_time_zone = timeminute;
	set_timezone_env(-m_n_time_zone);
}
/* 启动调整夏令时 */
static bool sumtime_adjust(void);
static void ntpclient_send_dst_change(sw_ntpstate_t state, uint32_t start_time, uint32_t end_time);
//初始化Client客户端
bool sw_ntpclient_init( char *server, int timezone, event_post_func event_post, void* handler )
{
	//网络时钟服务器IP地址
	strlcpy(m_ntpsvr, server, sizeof(m_ntpsvr));
	
	m_n_time_zone = timezone;
	//设置timezone环境变量。注意此函数需要传入的是 minutes west
	set_timezone_env( -m_n_time_zone );

	memset(&sumtime, 0, sizeof(sumtime));
	sumtime.b_sumtime = false;
	m_sumt_thread = NULL;

	m_event_post = event_post;
	m_event_handler = handler;
		
	m_b_give_signal = false;
	
	if( m_h_signal == NULL )
	 	m_h_signal = sw_signal_create(1,1);

	m_b_exit = false;

	if( m_h_thrd == NULL )
	{
		m_h_thrd = sw_thrd_open( "tNtpClient", 72, 0, 8192, ntp_client_proc,0,0 );
        if( NULL == m_h_thrd )
        {
            return false;
        }
		sw_thrd_resume(m_h_thrd);
	}	
	return true;
}

//释放Client客户端
void sw_ntpclient_exit()
{
	if( m_h_thrd != NULL )
	{
	    m_b_exit = true;
		m_b_give_signal = true;
		
	  	sw_signal_give( m_h_signal);
		sw_thrd_delay(50);		
			
		sw_thrd_close( m_h_thrd,100 );
		m_h_thrd = NULL;
	}
	if( m_h_signal != NULL )
	{
		sw_signal_destroy(m_h_signal);
		m_h_signal = NULL;
	}
	if ( m_sumt_thread != NULL )
	{
		sw_thrd_close( m_sumt_thread, 100 );
		m_sumt_thread = NULL;
	}
}

//设置网络时钟服务器地址
void sw_ntpclient_set_server( char *ntpserver )
{
	if( strcasecmp( ntpserver,m_ntpsvr) == 0 /*|| !sw_txtparser_is_address(ntpserver)*/ )
		return;
	strlcpy( m_ntpsvr,ntpserver,sizeof(m_ntpsvr));
	m_b_give_signal = true;
	
	sw_signal_give( m_h_signal);
}

//得到网络时钟服务器地址
char* sw_ntpclient_get_server()
{
	return m_ntpsvr;
}

//设置时区
void sw_ntpclient_set_timezone( int timezone )
{
	if( timezone >= -12*60 && timezone <= 13*60 )
	{
		if ( sumtime.b_sumtime )
		{/* 夏令时使用中不能修改时区,需要等到夏令结束才能自动调整 */
			MODULES_LOG_DEBUG("Current is Summer Time do not change timezone\n");
			m_n_time_zone = timezone;
			return;
		}
		if( m_n_time_zone != timezone )
		{
			//设置timezone环境变量。注意此函数需要传入的是 minutes west
			set_timezone_env( -timezone );
		
			m_n_time_zone = timezone;
			//ntpclient_send_dst_change();
			if( m_h_signal )
			{
				m_b_give_signal = true;
				sw_signal_give( m_h_signal);
			}
		}
	}
}

//取得系统当前设置的时区
int sw_ntpclient_get_timezone()
{
	if ( !sumtime.b_sumtime ) 
		return m_n_time_zone;
	else
		return sumtime.time_zone;
}

/*
 * @brief 设置夏令时的时区(分钟),开始时间,结束时间
 *
 * @param timezone 夏令时区,
		  start_time 夏令时开始时间
		  end_time 夏令时结束时间
*/
int sw_ntpclient_set_sumtime(int timezone, uint32_t start_time, uint32_t end_time)
{/* 开始时需要强制判断是否需要进入夏令时调整 */
	if( timezone < -12*60 || 13*60 < timezone || m_n_time_zone == timezone )	/* 时区不正确 */
		return -1;
	time_t 		utc;
	sumtime.flag = 1;
	sumtime.time_zone = timezone;
	sumtime.start_time = start_time;
	sumtime.end_time = end_time;
	time(&utc);
	if( ( (time_t)start_time ) <= utc && (utc < (time_t)end_time ) )
	{/* 修改为夏令时,在东西12区内 */
		set_timezone_env(-timezone);
		sumtime.b_sumtime = true;
		ntpclient_send_dst_change(SW_NTP_INTO_DST, sumtime.start_time, sumtime.end_time);
	}
	else if ( (time_t)sumtime.end_time < utc )
	{/* 结束夏令时 */
		sumtime.flag = 0;
		if ( sumtime.b_sumtime )
		{
			sumtime.b_sumtime = false;
			set_timezone_env(-m_n_time_zone);
			ntpclient_send_dst_change(SW_NTP_EXIT_DST, sumtime.start_time, sumtime.end_time);
		}
	}
	return 0;
}

/*
 * @brief 获取夏令时的时区(分钟),开始时间,结束时间
 *
 * @param timezone 夏令时区,
		  start_time 夏令时开始时间
		  end_time 夏令时结束时间
*/
int sw_ntpclient_get_sumtime(int* timezone, uint32_t* start_time, uint32_t* end_time)
{
	if( timezone==NULL || start_time==NULL || end_time==NULL )
		return -1;
	*timezone = sumtime.time_zone;
	*start_time = sumtime.start_time;
	*end_time = sumtime.end_time;
	return sumtime.flag;
}


void sw_ntpclient_resume()
{
	m_b_give_signal = true;
	
	sw_signal_give( m_h_signal);
}

void sw_ntpclient_set_localtime(int year,int month,int day,int hour,int minute,int second)
{
	struct tm  tm_time;
	time_t local_time;
	struct timespec res;
	
	memset(&tm_time,0,sizeof(struct tm));
	tm_time.tm_year = year -1900;
	tm_time.tm_mon  = month - 1;
	tm_time.tm_mday = day;
	tm_time.tm_hour = hour;
	tm_time.tm_min = minute;
	tm_time.tm_sec = second;
	tm_time.tm_isdst = -1;	
	local_time = mktime( &tm_time);
	res.tv_sec = local_time;
	res.tv_nsec = 0;
	clock_settime(CLOCK_REALTIME, &res);	
}

char*  sw_ntpclient_get_localtime()
{
	char* str_time = NULL;
	time_t local_time;
	time(&local_time);
	str_time = asctime( localtime(&local_time) );
	MODULES_LOG_DEBUG("%s\n",str_time);
	return str_time;
}


//设置到NTP-SERVER上获取时间的间隔，单位：second
void sw_ntpclient_set_interval( uint32_t interval )
{
	m_n_interval = interval*1000;
}


//得到NTP-SERVER上获取时间的间隔，单位：second
int sw_ntpclient_get_interval( uint32_t interval )
{
	return m_n_interval/1000;
}
 
/*covert time from seconds since 1900 1.1 to seconds since 1970 1.1*/
static unsigned long int sec_since_1970(unsigned long int  sec_since_1900)
{
	return sec_since_1900 - 2208988800u; 
}

/**
 * @note: 请不要使用这个函数，timezone可以调用set_timezone_env函数设到环境变量中
 */
static unsigned long int time_zone_modify(unsigned long int seconds, int time_zone)
{
	return seconds + (long int)time_zone*60;
}

static bool ntp_client_connect( char* ntpserver, int size, int timeout)
{
    if( ntpserver == NULL || ntpserver[0] == '\0' )
        return false;
	if(!sw_txtparser_is_address(ntpserver))
	{
        int ret;
        char result[1024];
        struct hostent *h=NULL;
		if ( strcasecmp(ntpserver, "http://") == 0 )
		{
			return false;
		}
		if ( strncasecmp(ntpserver, "http://", 7) == 0 )
			ntpserver += 7;
		ret=sw_network_gethostbyname(ntpserver,result,sizeof( result ),5000 );
		if( ret ==0 )
		{
			h=(struct hostent *)result;
			if( h->h_length >= 4 )
			{
                char buf[32] = {0};
                void *addr_list = h->h_addr_list[0];
                inet_ntop(AF_INET, addr_list,buf,sizeof(buf));
				snprintf(ntpserver,size,"%s",buf);
			}
			else
            {
                m_ntperrorCode = 102014;
                return false;
            }
        }
        else if(ret == -1)
        {
            m_ntperrorCode = 102013;
            return false;
        }
        else
        {
            m_ntperrorCode = 102014;
            return false;
        }
	}
	else
	{
		unsigned long srv_ip = inet_addr(ntpserver);
        m_ntperrorCode = 0;
		if( srv_ip == 0 || srv_ip == (unsigned long)-1 )
			return false;
	}

	MODULES_LOG_DEBUG("ntpserver:%s \n",ntpserver);
	//函数sntpcTimeGet允许ntpserver地址为空
	return ntp_time_get(ntpserver, timeout, NULL);
}
#ifdef SUPPORT_TTNET
static int m_ntp_get_count  = 10;
static int m_ntp_exponential= 10;
static bool ntp_client_proc( uint32_t wparam, uint32_t lparam )
{
	int interval = m_n_interval;
	char ntpserver[128] = {0};
	bool bSuccess = false;
	m_imm_ntp = false;
	if(m_b_exit)
		return false;
#if 0	
	if( sw_network_get_state() != NET_STATE_CONNECTED)
	{
		interval = 10000;
		goto WAIT;
	}
#endif
	int timeout = 5;//超时时间为5秒
	ntpserver[0] = '\0';
	ntpserver[sizeof(ntpserver)-1] = '\0';
	//暂时先获取参数,后续支持bootpara的格式后再修改
	sw_parameter_get( "ntpserver", ntpserver, sizeof(ntpserver) );
	//MODULES_LOG_DEBUG("[%s] ntpserver:%s, %d\n",__FUNCTION__,ntpserver, timeout);
	//if( !ntpserver[0] )
	//	continue;
	if( ntp_client_connect(ntpserver, sizeof(ntpserver), timeout) )
	{
		MODULES_LOG_DEBUG("[%s] ntp connect success\n",__FUNCTION__);
		/* 第三个参数表示是否画对话框 */
		if( m_event_post )
		{
			sw_event_t event;
			event.type = SW_NTP_EVENT;
			event.ntp.state = SW_NTP_SYNC_SUCCESS;
			m_event_post( m_event_handler, &event);
		}
		sw_turkey_time_correction();
		bSuccess = true;
		//break;
	}
	else
	{
		MODULES_LOG_DEBUG("ntp1 failed delay 5s!!\n");
		sw_thrd_delay(5000);//时间5s
	}
	if(!bSuccess)
	{
		ntpserver[0] = '\0';
		ntpserver[sizeof(ntpserver)-1] = '\0';
		sw_parameter_get( "ntpserver2", ntpserver, sizeof(ntpserver) );
		if( ntp_client_connect(ntpserver, sizeof(ntpserver), timeout) )
		{
			MODULES_LOG_DEBUG("[%s] ntp connect success\n",__FUNCTION__);
			/* 第三个参数表示是否画对话框 */
			if( m_event_post )
			{
				sw_event_t event;
				event.type = SW_NTP_EVENT;
				event.ntp.state = SW_NTP_SYNC_SUCCESS;
				m_event_post( m_event_handler, &event);
			}
			sw_turkey_time_correction();
			bSuccess = true;
			//break;
		}
	}
    MODULES_LOG_DEBUG("[%s][%d][%s] m_ntperrorCode:%d\n",__FILE__,__LINE__,__FUNCTION__,m_ntperrorCode);
	if( bSuccess )
	{
		if( !m_b_success_once && m_b_error_reported )	//多次连接失败后，第一次连接成功, 报告连接成功的消息
		{
//			sw_boot_post_message( SWMT_APP_SET_TIME, true, true , 0 );
            if( m_event_post )
            {
				sw_event_t event;
				event.type = SW_NTP_EVENT;
				event.ntp.state = SW_NTP_SYNC_SUCCESS;
            	m_event_post( m_event_handler, &event);
        	}
		}
		m_b_success_once = true;
		m_b_current_status = true;
        m_b_error_reported = false;
	}
	else
	{
		if( !m_b_error_reported )	//机顶盒上电开机时连接失败，报告连接失败的消息!m_b_success_once &&
		{
             if( m_event_post )
            {
				sw_event_t event;
				event.type = SW_NTP_EVENT;
                if( m_ntperrorCode == 102013 )
                    event.ntp.state = SW_NTP_DNS_OVERTIME;
                else if(m_ntperrorCode == 102014 )
                    event.ntp.state = SW_NTP_DNS_FAILED;
                else
				    event.ntp.state = SW_NTP_SYNC_FAILED;
            	m_event_post( m_event_handler, &event);
        	}
			m_b_error_reported = true;	 
            MODULES_LOG_DEBUG("#################ntp connect failed,m_b_error_reported:%d\n",m_b_error_reported);
		}
		m_b_current_status = false;
	}

	if( !m_b_current_status )
	{
		if( m_ntp_get_count>0 )
		{
			MODULES_LOG_DEBUG("ntp2 failed delay 5s!!\n");
			interval = 5000; //10次5秒间隔
		}
		else if( m_ntp_get_count==0 )
		{
			MODULES_LOG_DEBUG("ntp2 failed delay 10s!!\n");
			interval = m_ntp_exponential*1000;//第11次为10秒间隔
		}
		else if( m_ntp_get_count==-1 )
		{
			m_ntp_exponential = m_ntp_exponential*2;
			if(m_ntp_exponential > 640)
			{
				m_ntp_exponential=640;
			}
			MODULES_LOG_DEBUG("ntp2 failed delay %ds!!\n", m_ntp_exponential);
			interval = m_ntp_exponential*1000;	//后续如果没成功，则10的2的指数递增，如果到达640s时候则做一直维持640s
		}
	}
	if(m_ntp_get_count>-1) //三种状态 <10 && ==0 && ==-1
	{
		m_ntp_get_count--;
	}
    MODULES_LOG_DEBUG("m_b_success_once:%d,m_b_error_reported:%d,m_b_current_status:%d,interval:%d\n",m_b_success_once,m_b_error_reported,m_b_current_status,interval);
WAIT:
	while(interval > 0)
	{
		time_t utc;
		if ( sumtime.flag == 1 )
		{
			time(&utc);
			if ( sumtime.b_sumtime && utc >= (time_t)(sumtime.end_time - 66 ) )//10+5+8+15+5+8+15
			{/* 需要的是一个同步周期 */
				sumtime_adjust();
			}
			else if ( !sumtime.b_sumtime && ((time_t)sumtime.start_time - 66) < utc && utc < (time_t)sumtime.end_time )
			{
				sumtime_adjust();
			}
		}
		if(m_h_signal && (!m_imm_ntp))
			sw_signal_wait( m_h_signal,5000);
		if (m_imm_ntp)
			interval = 0;
		else
			interval -= 5000;
		if(m_b_give_signal)
		{
			m_b_give_signal = false;
            if(m_b_success_once)
                m_b_success_once = false;
			break;
		}

	}

	return true;
}
#else
#ifdef SUPPORT_TURKCELL
static int m_ntp_start_once = 0;/* 开机启动第一次同步 */
static bool ntp_client_proc( uint32_t wparam, uint32_t lparam )
{
	int i, j;
	int interval = m_n_interval;
	char ntpserver[128] = {0};
	bool bSuccess = false;
	if (!m_ntp_start_once)
	{/* 第一次同步时占用信号量，便于以后使用信号量来完成线程阻塞相应的时间 */
		sw_signal_wait( m_h_signal, -1);
		m_ntp_start_once = 1;
	}
	if(m_b_exit)
		return false;
	m_imm_ntp = false;
#if 0
	if( sw_network_get_state() != NET_STATE_CONNECTED)
	{
		interval = 10000;
		goto WAIT;
	}
#endif

	for( i=0; i<2; i++ )	//重试两次
	{
		int timeout[2] = {2, 4}; //根据V1R5C03规格，重试超时时间分别为5s,8s,15s
		unsigned int st_time = 0;
		unsigned int margin_time = 0;
		for( j=0; j<3; j++ )	//支持三个时间服务器
		{
			st_time = sw_thrd_get_tick();
			ntpserver[0] = '\0';
			ntpserver[sizeof(ntpserver)-1] = '\0';
			if( j==0 )
				strlcpy( ntpserver, m_ntpsvr, sizeof(ntpserver) );
			else if( j==1 )
				sw_parameter_get( "ntpserver2", ntpserver, sizeof(ntpserver) );
			else
				sw_parameter_get( "ntpserver3", ntpserver, sizeof(ntpserver) );
            MODULES_LOG_DEBUG("[%s] ntpserver:%s, timeout[%d]:%d\n",__FUNCTION__,ntpserver, i, timeout[i]);
			if( !ntpserver[0] )
				continue;
			if( ntp_client_connect(ntpserver, sizeof(ntpserver), timeout[i]) )
			{
                MODULES_LOG_DEBUG("[%s] ntp connect success\n",__FUNCTION__);
				/* 第三个参数表示是否画对话框 */
                if( m_event_post )
                {
					sw_event_t event;
					event.type = SW_NTP_EVENT;
					event.ntp.state = SW_NTP_SYNC_SUCCESS;
                    m_event_post( m_event_handler, &event);
                }
				bSuccess = true;
				break;
			}
			/* 出错时等待超时 */
			margin_time = sw_thrd_get_tick() - st_time;
			if (margin_time < (timeout[i] *1000))
			{
				margin_time = timeout[i] *1000 - margin_time;
				sw_signal_wait(m_h_signal, margin_time);
			}
		}
		if( bSuccess )
			break;
	}
    MODULES_LOG_DEBUG("[%s][%d][%s] m_ntperrorCode:%d\n",__FILE__,__LINE__,__FUNCTION__,m_ntperrorCode);
	if( bSuccess )
	{
		if( !m_b_success_once && m_b_error_reported )	//多次连接失败后，第一次连接成功, 报告连接成功的消息
		{
//			sw_boot_post_message( SWMT_APP_SET_TIME, true, true , 0 );
            if( m_event_post )
            {
				sw_event_t event;
				event.type = SW_NTP_EVENT;
				event.ntp.state = SW_NTP_SYNC_SUCCESS;
            	m_event_post( m_event_handler, &event);
        	}
		}
		m_b_success_once = true;
		m_b_current_status = true;
        m_b_error_reported = false;
	}
	else
	{
		if( !m_b_error_reported )	//机顶盒上电开机时连接失败，报告连接失败的消息!m_b_success_once &&
		{
             if( m_event_post )
            {
				sw_event_t event;
				event.type = SW_NTP_EVENT;
                if( m_ntperrorCode == 102013 )
                    event.ntp.state = SW_NTP_DNS_OVERTIME;
                else if(m_ntperrorCode == 102014 )
                    event.ntp.state = SW_NTP_DNS_FAILED;
                else
				    event.ntp.state = SW_NTP_SYNC_FAILED;
            	m_event_post( m_event_handler, &event);
        	}
			m_b_error_reported = true;	 
            MODULES_LOG_DEBUG("#################ntp connect failed,m_b_error_reported:%d\n",m_b_error_reported);
		}
		m_b_current_status = false;
	}

	if( !m_b_current_status )
	{
			interval = 10000;//时同步失败，10s重新同步
	}

    MODULES_LOG_DEBUG("m_b_success_once:%d,m_b_error_reported:%d,m_b_current_status:%d,interval:%d\n",m_b_success_once,m_b_error_reported,m_b_current_status,interval);
WAIT:
	while(interval > 0)
	{
		time_t utc;
		if ( sumtime.flag == 1 )
		{
			time(&utc);
			if ( sumtime.b_sumtime && utc >= (time_t)(sumtime.end_time - 66 ) )//10+5+8+15+5+8+15
			{/* 需要的是一个同步周期 */
				sumtime_adjust();
			}
			else if ( !sumtime.b_sumtime && ((time_t)sumtime.start_time - 66) < utc && utc < (time_t)sumtime.end_time )
			{
				sumtime_adjust();
			}
		}
		if(m_h_signal && (!m_imm_ntp))
			sw_signal_wait( m_h_signal,10000);
		if (m_imm_ntp)
			interval = 0;
		else
			interval -= 10000;
		if(m_b_give_signal)
		{
			m_b_give_signal = false;
            if(m_b_success_once)
                m_b_success_once = false;
			break;
		}

	}

	return true;
}
#else
static bool ntp_client_proc(uint32_t wparam, uint32_t lparam)
{
	int i, j;
	int interval = m_n_interval;
	char ntpserver[128] = {0};
	bool bSuccess = false;

	if(m_b_exit)
		return false;
	m_imm_ntp = false;
#if 0
	if( sw_network_get_state() != NET_STATE_CONNECTED)
	{
		interval = 10000;
		goto WAIT;
	}
#endif

	for( i=0; i<3; i++ )	//重试三次
	{
		int timeout[3] = {5,8,15}; //根据V1R5C03规格，重试超时时间分别为5s,8s,15s
		for( j=0; j<3; j++ )	//支持三个时间服务器
		{
			ntpserver[0] = '\0';
			ntpserver[sizeof(ntpserver)-1] = '\0';
			if( j==0 )
				strlcpy( ntpserver, m_ntpsvr, sizeof(ntpserver) );
			else if( j==1 )
				sw_parameter_get( "ntpserver2", ntpserver, sizeof(ntpserver) );
			else
				sw_parameter_get( "ntpserver3", ntpserver, sizeof(ntpserver) );
            MODULES_LOG_DEBUG("[%s] ntpserver:%s, timeout[%d]:%d\n",__FUNCTION__,ntpserver, i, timeout[i]);
			if( !ntpserver[0] )
				continue;

			if( ntp_client_connect(ntpserver, sizeof(ntpserver), timeout[i]) )
			{
                MODULES_LOG_DEBUG("[%s] ntp connect success\n",__FUNCTION__);
				/* 第三个参数表示是否画对话框 */
                if( m_event_post )
                {
					sw_event_t event;
					event.type = SW_NTP_EVENT;
					event.ntp.state = SW_NTP_SYNC_SUCCESS;
					event.ntp.start_time = m_new_time + 8*3600;
                    m_event_post( m_event_handler, &event);
                }
				bSuccess = true;
				break;
			}
			else
			{
			    if( m_event_post )
                {
					sw_event_t event;
					event.type = SW_NTP_EVENT;
					event.ntp.state = SW_NTP_SYNC_FAILED;
                    m_event_post( m_event_handler, &event);
                }
			}
		}
		if( bSuccess )
			break;
	}
#if 0
    MODULES_LOG_DEBUG("[%s][%d][%s] m_ntperrorCode:%d\n",__FILE__,__LINE__,__FUNCTION__,m_ntperrorCode);
	if( bSuccess )
	{
		if( !m_b_success_once && m_b_error_reported )	//多次连接失败后，第一次连接成功, 报告连接成功的消息
		{
//			sw_boot_post_message( SWMT_APP_SET_TIME, true, true , 0 );
            if( m_event_post )
            {
				sw_event_t event;
				event.type = SW_NTP_EVENT;
				event.ntp.state = SW_NTP_SYNC_SUCCESS;
            	m_event_post( m_event_handler, &event);
        	}
		}
		m_b_success_once = true;
		m_b_current_status = true;
        m_b_error_reported = false;
	}
	else
	{
		if( !m_b_error_reported )	//机顶盒上电开机时连接失败，报告连接失败的消息!m_b_success_once &&
		{
             if( m_event_post )
            {
				sw_event_t event;
				event.type = SW_NTP_EVENT;
                if( m_ntperrorCode == 102013 )
                    event.ntp.state = SW_NTP_DNS_OVERTIME;
                else if(m_ntperrorCode == 102014 )
                    event.ntp.state = SW_NTP_DNS_FAILED;
                else
				    event.ntp.state = SW_NTP_SYNC_FAILED;
            	m_event_post( m_event_handler, &event);
        	}
			m_b_error_reported = true;	 
            MODULES_LOG_DEBUG("#################ntp connect failed,m_b_error_reported:%d\n",m_b_error_reported);
		}
		m_b_current_status = false;
	}
	if( !m_b_current_status )
	{
		interval = 600 * 1000;	//上次没有成功，过10分钟再尝试

	}
#endif

    MODULES_LOG_DEBUG("m_b_success_once:%d,m_b_error_reported:%d,m_b_current_status:%d,interval:%d\n",m_b_success_once,m_b_error_reported,m_b_current_status,interval);
WAIT:
	while(interval > 0)
	{
		time_t utc;
		if ( sumtime.flag == 1 )
		{
			time(&utc);
			if ( sumtime.b_sumtime && utc >= (time_t)(sumtime.end_time - 66 ) )//10+5+8+15+5+8+15
			{/* 需要的是一个同步周期 */
				sumtime_adjust();
			}
			else if ( !sumtime.b_sumtime && ((time_t)sumtime.start_time - 66) < utc && utc < (time_t)sumtime.end_time )
			{
				sumtime_adjust();
			}
		}
		if(m_h_signal && (!m_imm_ntp))
			sw_signal_wait( m_h_signal,3000);
		if (m_imm_ntp)
			interval = 0;
		else
			interval -= 3000;
		if(m_b_give_signal)
		{
			m_b_give_signal = false;
            if(m_b_success_once)
                m_b_success_once = false;
			break;
		}

	}

	return true;
}
#endif
#endif

/* ********************************************************************* */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>     /* gethostbyname */
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/time.h>

typedef u_int32_t __u32;
#include <sys/timex.h>

#define NTP_PORT (123)
#define JAN_1970        0x83aa7e80      /* 2208988800 1970 - 1900 in seconds */

/* How to multiply by 4294.967296 quickly (and not quite exactly)
 * without using floating point or greater than 32-bit integers.
 * If you want to fix the last 12 microseconds of error, add in
 * (2911*(x))>>28)
 */
#define NTPFRAC(x) ( 4294*(x) + ( (1981*(x))>>11 ) )

/* The reverse of the above, needed if we want to set our microsecond
 * clock (via settimeofday) based on the incoming time in NTP format.
 * Basically exact.
 */
#define USEC(x) ( ( (x) >> 12 ) - 759 * ( ( ( (x) >> 10 ) + 32768 ) >> 16 ) )

/* Converts NTP delay and dispersion, apparently in seconds scaled
 * by 65536, to microseconds.  RFC1305 states this time is in seconds,
 * doesn't mention the scaling.
 * Should somehow be the same as 1000000 * x / 65536
 */
#define sec2u(x) ( (x) * 15.2587890625 )

struct ntptime {
	unsigned int coarse;
	unsigned int fine;
};

static int debug = 1;
/* variables with file scope
 * (I know, bad form, but this is a short program) */
static uint32_t incoming_word[325];
#define incoming ((char *) incoming_word)
#define sizeof_incoming (sizeof(incoming_word)*sizeof(uint32_t))
static struct timeval time_of_send;
static int live=0;


#define RING_SIZE 16
#define MAX_CORRECT 250   /* ppm change to system clock */
#define MAX_C ((MAX_CORRECT)*65536)
struct datum {
	unsigned int absolute;
	double skew;
	double errorbar;
	int freq;
	/* s.s.min and s.s.max (skews) are "corrected" to what they would
	 * have been if freq had been constant at its current value during
	 * the measurements.
	 */
	union {
		struct { double min; double max; } s;
		double ss[2];
	} s;
	/*
	double smin;
	double smax;
	 */
} d_ring[RING_SIZE];

struct _seg {
	double slope;
	double offset;
} maxseg[RING_SIZE+1], minseg[RING_SIZE+1];


/* draw a line from a to c, what the offset is of that line
 * where that line matches b's slope coordinate.
 */
/* static double interpolate(struct _seg *a, struct _seg *b, struct _seg *c) */
/* { */
/* 	double x, y; */
/* 	x = (b->slope - a->slope) / (c->slope  - a->slope) ; */
/* 	y =         a->offset + x * (c->offset - a->offset); */
/* 	return y; */
/* } */

static int next_up(int i) { int r = i+1; if (r>=RING_SIZE) r=0; return r;}
static int next_dn(int i) { int r = i-1; if (r<0) r=RING_SIZE-1; return r;}

/* Looks at the line segments that start at point j, that end at
 * all following points (ending at index rp).  The initial point
 * is on curve s0, the ending point is on curve s1.  The curve choice
 * (s.min vs. s.max) is based on the index in ss[].  The scan
 * looks for the largest (sign=0) or smallest (sign=1) slope.
 */
static int search(int rp, int j, int s0, int s1, int sign, struct _seg *answer)
{
	double dt, slope;
	int n, nextj=0, cinit=1;
	for (n=next_up(j); n!=next_up(rp); n=next_up(n)) {
		if (0 && debug) MODULES_LOG_DEBUG("d_ring[%d].s.ss[%d]=%f d_ring[%d].s.ss[%d]=%f\n",
			n, s0, d_ring[n].s.ss[s0], j, s1, d_ring[j].s.ss[s1]);
		dt = d_ring[n].absolute - d_ring[j].absolute;
		slope = (d_ring[n].s.ss[s0] - d_ring[j].s.ss[s1]) / dt;
		if (0 && debug) MODULES_LOG_DEBUG("slope %d%d%d [%d,%d] = %f\n",
			s0, s1, sign, j, n, slope);
		if (cinit || (slope < answer->slope) ^ sign) {
			answer->slope = slope;
			answer->offset = d_ring[n].s.ss[s0] +
				slope*(d_ring[rp].absolute - d_ring[n].absolute);
			cinit = 0;
			nextj = n;
		}
	}
	return nextj;
}

/* Pseudo-class for finding consistent frequency shift */
#define MIN_INIT 20
struct _polygon {
	double l_min;
	double r_min;
} df;

static void polygon_reset(void)
{
	df.l_min = MIN_INIT;
	df.r_min = MIN_INIT;
}

static double find_df(int *flag)
{
	if (df.l_min == 0.0) {
		if (df.r_min == 0.0) {
			return 0.0;   /* every point was OK */
		} else {
			return -df.r_min;
		}
	} else {
		if (df.r_min == 0.0) {
			return df.l_min;
		} else {
			if (flag) *flag=1;
			return 0.0;   /* some points on each side,
			               * or no data at all */
		}
	}
}

/* Finds the amount of delta-f required to move a point onto a
 * target line in delta-f/delta-t phase space.  Any line is OK
 * as long as it's not convex and never returns greater than
 * MIN_INIT. */
static double find_shift(double slope, double offset)
{
	double shift  = slope - offset/600.0;
	double shift2 = slope + 0.3 - offset/6000.0;
	if (shift2 < shift) shift = shift2;
	if (debug) MODULES_LOG_DEBUG("find_shift %f %f -> %f\n", slope, offset, shift);
	if (shift  < 0) return 0.0;
	return shift;
}

static void polygon_point(struct _seg *s)
{
	double l, r;
	if (debug) MODULES_LOG_DEBUG("loop %f %f\n", s->slope, s->offset);
	l = find_shift(- s->slope,   s->offset);
	r = find_shift(  s->slope, - s->offset);
	if (l < df.l_min) df.l_min = l;
	if (r < df.r_min) df.r_min = r;
	if (debug) MODULES_LOG_DEBUG("constraint left:  %f %f \n", l, df.l_min);
	if (debug) MODULES_LOG_DEBUG("constraint right: %f %f \n", r, df.r_min);
}

/* Something like linear feedback to be used when we are "close" to
 * phase lock.  Not really used at the moment:  the logic in find_df()
 * never sets the flag. */
static double find_df_center(struct _seg *min, struct _seg *max, double gross_df)
{
	const double crit_time=1000.0;
	double slope  = 0.5 * (max->slope  + min->slope)+gross_df;
	double dslope =       (max->slope  - min->slope);
	double offset = 0.5 * (max->offset + min->offset);
	double doffset =      (max->offset - min->offset);
	double delta1 = -offset/ 600.0 - slope;
	double delta2 = -offset/1800.0 - slope;
	double delta  = 0.0;
	double factor = crit_time/(crit_time+doffset+dslope*1200.0);
	if (offset <  0 && delta2 > 0) delta = delta2;
	if (offset <  0 && delta1 < 0) delta = delta1;
	if (offset >= 0 && delta1 > 0) delta = delta1;
	if (offset >= 0 && delta2 < 0) delta = delta2;
	if (max->offset < -crit_time || min->offset > crit_time) return 0.0;
	if (debug) MODULES_LOG_DEBUG("find_df_center %f %f\n", delta, factor);
	return factor*delta;
}

static int contemplate_data(unsigned int absolute, double skew, double errorbar, int freq)
{
	/*  Here is the actual phase lock loop.
	 *  Need to keep a ring buffer of points to make a rational
	 *  decision how to proceed.  if (debug) print a lot.
	 */
	static int rp=0, valid=0;
	int both_sides_now=0;
	int j, n, c, max_avail, min_avail, dinit;
	int nextj=0;	/* initialization not needed; but gcc can't figure out my logic */
	double cum;
	struct _seg check, save_min, save_max;
	double last_slope;
	int delta_freq;
	double delta_f;
	int inconsistent=0, max_imax, max_imin=0, min_imax, min_imin=0;
	int computed_freq=freq;

	if (debug) MODULES_LOG_DEBUG("xontemplate %u %.1f %.1f %d\n",absolute,skew,errorbar,freq);
	d_ring[rp].absolute = absolute;
	d_ring[rp].skew     = skew;
	d_ring[rp].errorbar = errorbar - 800.0;   /* quick hack to speed things up */
	d_ring[rp].freq     = freq;

	if (valid<RING_SIZE) ++valid;
	if (valid==RING_SIZE) {
		/*
		 * Pass 1: correct for wandering freq's */
		cum = 0.0;
		if (debug) MODULES_LOG_DEBUG("\n");
		for (j=rp; ; j=n) {
			d_ring[j].s.s.max = d_ring[j].skew - cum + d_ring[j].errorbar;
			d_ring[j].s.s.min = d_ring[j].skew - cum - d_ring[j].errorbar;
			if (debug) MODULES_LOG_DEBUG("hist %d %d %f %f %f\n",j,d_ring[j].absolute-absolute,
				cum,d_ring[j].s.s.min,d_ring[j].s.s.max);
			n=next_dn(j);
			if (n == rp) break;
			/* Assume the freq change took place immediately after
			 * the data was taken; this is valid for the case where
			 * this program was responsible for the change.
			 */
			cum = cum + (d_ring[j].absolute-d_ring[n].absolute) *
				(double)(d_ring[j].freq-freq)/65536;
		}
		/*
		 * Pass 2: find the convex down envelope of s.max, composed of
		 * line segments in s.max vs. absolute space, which are
		 * points in freq vs. dt space.  Find points in order of increasing
		 * slope == freq */
		dinit=1; last_slope=-2*MAX_CORRECT;
		for (c=1, j=next_up(rp); ; j=nextj) {
			nextj = search(rp, j, 1, 1, 0, &maxseg[c]);
			        search(rp, j, 0, 1, 1, &check);
			if (check.slope < maxseg[c].slope && check.slope > last_slope &&
			    (dinit || check.slope < save_min.slope)) {dinit=0; save_min=check; }
			if (debug) MODULES_LOG_DEBUG("maxseg[%d] = %f *x+ %f\n",
				 c, maxseg[c].slope, maxseg[c].offset);
			last_slope = maxseg[c].slope;
			c++;
			if (nextj == rp) break;
		}
		if (dinit==1) inconsistent=1;
		if (debug && dinit==0) MODULES_LOG_DEBUG ("mincross %f *x+ %f\n", save_min.slope, save_min.offset);
		max_avail=c;
		/*
		 * Pass 3: find the convex up envelope of s.min, composed of
		 * line segments in s.min vs. absolute space, which are
		 * points in freq vs. dt space.  These points are found in
		 * order of decreasing slope. */
		dinit=1; last_slope=+2*MAX_CORRECT;
		for (c=1, j=next_up(rp); ; j=nextj) {
			nextj = search(rp, j, 0, 0, 1, &minseg[c]);
			        search(rp, j, 1, 0, 0, &check);
			if (check.slope > minseg[c].slope && check.slope < last_slope &&
			    (dinit || check.slope < save_max.slope)) {dinit=0; save_max=check; }
			if (debug) MODULES_LOG_DEBUG("minseg[%d] = %f *x+ %f\n",
				 c, minseg[c].slope, minseg[c].offset);
			last_slope = minseg[c].slope;
			c++;
			if (nextj == rp) break;
		}
		if (dinit==1) inconsistent=1;
		if (debug && dinit==0) MODULES_LOG_DEBUG ("maxcross %f *x+ %f\n", save_max.slope, save_max.offset);
		min_avail=c;
		/*
		 * Pass 4: splice together the convex polygon that forms
		 * the envelope of slope/offset coordinates that are consistent
		 * with the observed data.  The order of calls to polygon_point
		 * doesn't matter for the frequency shift determination, but
		 * the order chosen is nice for visual display. */
		if (!inconsistent) {
		polygon_reset();
		polygon_point(&save_min);
		for (dinit=1, c=1; c<max_avail; c++) {
			if (dinit && maxseg[c].slope > save_min.slope) {
				max_imin = c-1;
				maxseg[max_imin] = save_min;
				dinit = 0;
			}
			if (maxseg[c].slope > save_max.slope)
				break;
			if (dinit==0) polygon_point(&maxseg[c]);
		}
		if (dinit && debug) MODULES_LOG_DEBUG("found maxseg vs. save_min inconsistency\n");
		if (dinit) inconsistent=1;
		max_imax = c;
		maxseg[max_imax] = save_max;

		polygon_point(&save_max);
		for (dinit=1, c=1; c<min_avail; c++) {
			if (dinit && minseg[c].slope < save_max.slope) {
				max_imin = c-1;
				minseg[min_imin] = save_max;
				dinit = 0;
			}
			if (minseg[c].slope < save_min.slope)
				break;
			if (dinit==0) polygon_point(&minseg[c]);
		}
		if (dinit && debug) MODULES_LOG_DEBUG("found minseg vs. save_max inconsistency\n");
		if (dinit) inconsistent=1;
		min_imax = c;
		minseg[min_imax] = save_max;

		/* not needed for analysis, but shouldn't hurt either */
		if (debug) polygon_point(&save_min);
		} /* !inconsistent */

		/*
		 * Pass 5: decide on a new freq */
		if (inconsistent) {
			MODULES_LOG_DEBUG("# inconsistent\n");
		} else {
			delta_f = find_df(&both_sides_now);
			if (debug) MODULES_LOG_DEBUG("find_df() = %e\n", delta_f);
			delta_f += find_df_center(&save_min,&save_max, delta_f);
			delta_freq = delta_f*65536+.5;
			if (debug) MODULES_LOG_DEBUG("delta_f %f  delta_freq %d  bsn %d\n", delta_f, delta_freq, both_sides_now);
			computed_freq -= delta_freq;
			MODULES_LOG_DEBUG ("# box [( %.3f , %.1f ) ",  save_min.slope, save_min.offset);
			MODULES_LOG_DEBUG (      " ( %.3f , %.1f )] ", save_max.slope, save_max.offset);
			MODULES_LOG_DEBUG (" delta_f %.3f  computed_freq %d\n", delta_f, computed_freq);

			if (computed_freq < -MAX_C) computed_freq=-MAX_C;
			if (computed_freq >  MAX_C) computed_freq= MAX_C;
		}
	}
	rp = (rp+1)%RING_SIZE;
	return computed_freq;
}

static int get_current_freq(void)
{
	/* OS dependent routine to get the current value of clock frequency.
	 */
#ifdef linux
	struct timex txc;
	txc.modes=0;
	if (__adjtimex(&txc) < 0) {
		perror("adjtimex"); return -1;
	}
	return txc.freq;
#else
	return 0;
#endif
}

static int set_freq(int new_freq)
{
	/* OS dependent routine to set a new value of clock frequency.
	 */
#ifdef linux
	struct timex txc;
	txc.modes = ADJ_FREQUENCY;
	txc.freq = new_freq;
	if (__adjtimex(&txc) < 0) {
		perror("adjtimex"); return -1;;
	}
	return txc.freq;
#else
	return 0;
#endif
}

static void send_packet(int usd)
{
	__u32 data[12];
	struct timeval now;
#define LI 0
#define VN 3
#define MODE 3
#define STRATUM 0
#define POLL 4 
#define PREC -6

	if (debug) MODULES_LOG_DEBUG("Sending ...\n");
	if (sizeof(data) != 48) {
		MODULES_LOG_DEBUG("size error\n");
		return;
	}
	bzero((char *) data,sizeof(data));
	data[0] = htonl (
		( LI << 30 ) | ( VN << 27 ) | ( MODE << 24 ) |
		( STRATUM << 16) | ( POLL << 8 ) | ( PREC & 0xff ) );
	data[1] = htonl(1<<16);  /* Root Delay (seconds) */
	data[2] = htonl(1<<16);  /* Root Dispersion (seconds) */
	gettimeofday(&now,NULL);
	data[10] = htonl(now.tv_sec + JAN_1970); /* Transmit Timestamp coarse */
	data[11] = htonl(NTPFRAC(now.tv_usec));  /* Transmit Timestamp fine   */
	if ( send(usd,data,48,0) < 0 )
	{
		MODULES_LOG_DEBUG("send data error\n");
		return;
	}
	time_of_send=now;
}

static void get_packet_timestamp(int usd, struct ntptime *udp_arrival_ntp)
{
	struct timeval udp_arrival;
#ifdef _PRECISION_SIOCGSTAMP
	if ( ioctl(usd, SIOCGSTAMP, &udp_arrival) < 0 ) {
		perror("ioctl-SIOCGSTAMP");
		gettimeofday(&udp_arrival,NULL);
	}
#else
	gettimeofday(&udp_arrival,NULL);
#endif
	udp_arrival_ntp->coarse = udp_arrival.tv_sec + JAN_1970;
	udp_arrival_ntp->fine   = NTPFRAC(udp_arrival.tv_usec);
}

static void check_source(int data_len, struct sockaddr *sa_source, int sa_len)
{
	/* This is where one could check that the source is the server we expect */
	if (debug) {
		struct sockaddr_in *sa_in=(struct sockaddr_in *)sa_source;
		MODULES_LOG_DEBUG("packet of length %d received\n",data_len);
		if (sa_source->sa_family==AF_INET) {
			char dst[32];
			memset(dst, 0, sizeof(dst));
			inet_ntop(AF_INET, &sa_in->sin_addr, dst, sizeof(dst));
			MODULES_LOG_DEBUG("Source: INET Port %d host %s\n",
				ntohs(sa_in->sin_port),dst);
		} else {
			MODULES_LOG_DEBUG("Source: Address family %d\n",sa_source->sa_family);
		}
	}
}

static double ntpdiff( struct ntptime *start, struct ntptime *stop)
{
	int a;
	unsigned int b;
	a = stop->coarse - start->coarse;
	if (stop->fine >= start->fine) {
		b = stop->fine - start->fine;
	} else {
		b = start->fine - stop->fine;
		b = ~b;
		a -= 1;
	}
	
	return a*1.e6 + b * (1.e6/4294967296.0);
}

/* Does more than print, so this name is bogus.
 * It also makes time adjustments, both sudden (-s)
 * and phase-locking (-l).  */
/* return value is number of microseconds uncertainty in answer */
static int rfc1305print(uint32_t *data, struct ntptime *arrival, struct timeval *tv)
{
/* straight out of RFC-1305 Appendix A */
	int li, vn, mode, stratum, poll, prec;
	int delay, disp, refid;
	struct ntptime reftime, orgtime, rectime, xmttime;
	double el_time,st_time,skew1,skew2;
	int freq;

#define Data(i) ntohl(((uint32_t *)data)[i])
	li      = Data(0) >> 30 & 0x03;
	vn      = Data(0) >> 27 & 0x07;
	mode    = Data(0) >> 24 & 0x07;
	stratum = Data(0) >> 16 & 0xff;
	poll    = Data(0) >>  8 & 0xff;
	prec    = Data(0)       & 0xff;
	if (prec & 0x80) prec|=0xffffff00;
	delay   = Data(1);
	disp    = Data(2);
	refid   = Data(3);
	reftime.coarse = Data(4);
	reftime.fine   = Data(5);
	orgtime.coarse = Data(6);
	orgtime.fine   = Data(7);
	rectime.coarse = Data(8);
	rectime.fine   = Data(9);
	xmttime.coarse = Data(10);
	xmttime.fine   = Data(11);
#undef Data

	if( tv )	/* 输出时间服务器的时间 */
	{
		tv->tv_sec  = xmttime.coarse - JAN_1970;
		tv->tv_usec = USEC(xmttime.fine);
	}
	/*else*/{   /* you'd better be root, or ntpclient will crash! */
		struct timeval tv_set;
		/* it would be even better to subtract half the slop */
		tv_set.tv_sec  = xmttime.coarse - JAN_1970;
		/* divide xmttime.fine by 4294.967296 */
		tv_set.tv_usec = USEC(xmttime.fine);

		m_new_time = xmttime.coarse - JAN_1970;

#if 0
		if (settimeofday(&tv_set, NULL)<0) {
			perror("settimeofday");
			return -1;;
		}
#endif

		if (debug) {
			MODULES_LOG_DEBUG("set time to %lu.%.6lu\n", tv_set.tv_sec, tv_set.tv_usec);
		}
	}

	if (debug) {
	MODULES_LOG_DEBUG("LI=%d  VN=%d  Mode=%d  Stratum=%d  Poll=%d  Precision=%d\n",
		li, vn, mode, stratum, poll, prec);
	MODULES_LOG_DEBUG("Delay=%.1f  Dispersion=%.1f  Refid=%u.%u.%u.%u\n",
		sec2u(delay),sec2u(disp),
		refid>>24&0xff, refid>>16&0xff, refid>>8&0xff, refid&0xff);
	MODULES_LOG_DEBUG("Reference %u.%.10u\n", reftime.coarse, reftime.fine);
	MODULES_LOG_DEBUG("Originate %u.%.10u\n", orgtime.coarse, orgtime.fine);
	MODULES_LOG_DEBUG("Receive   %u.%.10u\n", rectime.coarse, rectime.fine);
	MODULES_LOG_DEBUG("Transmit  %u.%.10u\n", xmttime.coarse, xmttime.fine);
	MODULES_LOG_DEBUG("Our recv  %u.%.10u\n", arrival->coarse, arrival->fine);
	}
	el_time=ntpdiff(&orgtime,arrival);   /* elapsed */
	st_time=ntpdiff(&rectime,&xmttime);  /* stall */
	skew1=ntpdiff(&orgtime,&rectime);
	skew2=ntpdiff(&xmttime,arrival);
	freq=get_current_freq();
	if (debug) {
	MODULES_LOG_DEBUG("Total elapsed: %9.2f\n"
	       "Server stall:  %9.2f\n"
	       "Slop:          %9.2f\n",
		el_time, st_time, el_time-st_time);
	MODULES_LOG_DEBUG("Skew:          %9.2f\n"
	       "Frequency:     %9d\n"
	       " day   second     elapsed    stall     skew  dispersion  freq\n",
		(skew1-skew2)/2, freq);
	}
	/* Not the ideal order for printing, but we want to be sure
	 * to do all the time-sensitive thinking (and time setting)
	 * before we start the output, especially fflush() (which
	 * could be slow).  Of course, if debug is turned on, speed
	 * has gone down the drain anyway. */
	if (live) {
		int new_freq;
		new_freq = contemplate_data(arrival->coarse, (skew1-skew2)/2,
			el_time+sec2u(disp), freq);
		if (!debug && new_freq != freq) set_freq(new_freq);
	}
	MODULES_LOG_DEBUG("%d %.5d.%.3d  %8.1f %8.1f  %8.1f %8.1f %9d\n",
		arrival->coarse/86400, arrival->coarse%86400,
		arrival->fine/4294967, el_time, st_time,
		(skew1-skew2)/2, sec2u(disp), freq);
/* 	fflush(stdout); */
	return(el_time-st_time);
}


static void stuff_net_addr(struct in_addr *p, char *hostname)
{
	struct hostent *ntpserver;
	ntpserver=gethostbyname(hostname);
	if (ntpserver == NULL) {
		herror(hostname);
		return;;
	}
	if (ntpserver->h_length != 4) {
		MODULES_LOG_DEBUG("oops %d\n",ntpserver->h_length);
		return;;
	}
	memcpy(&(p->s_addr),ntpserver->h_addr_list[0],4);
}

static int setup_receive(int usd, unsigned int interface, short port)
{
	struct sockaddr_in sa_rcvr;
	int reuse = 1;
	
	bzero((char *) &sa_rcvr, sizeof(sa_rcvr));
	sa_rcvr.sin_family=AF_INET;
	sa_rcvr.sin_addr.s_addr=htonl(interface);
	sa_rcvr.sin_port=htons(port);

	if( -1 == setsockopt( usd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) )
	{
		MODULES_LOG_DEBUG("setsockopt error\n");
		return -1;
	}
	
	if(bind(usd,(struct sockaddr *) &sa_rcvr,sizeof(sa_rcvr)) == -1) {
		MODULES_LOG_DEBUG("could not bind to udp port %d\n",port);
		perror("bind");
		return -1;;
	}
	listen(usd,3);

	return 0;
}

static int setup_transmit(int usd, char *host, short port)
{
	struct sockaddr_in sa_dest;
	bzero((char *) &sa_dest, sizeof(sa_dest));
	sa_dest.sin_family=AF_INET;
	
	stuff_net_addr(&(sa_dest.sin_addr),host);
	sa_dest.sin_port=htons(port);
	if (connect(usd,(struct sockaddr *)&sa_dest,sizeof(sa_dest))==-1)
	  {perror("connect");return -1;}

	return 0;
}


static int primary_loop(int usd, int num_probes, int interval, int goodness, struct timeval *tv)
{
	fd_set fds;
	struct sockaddr sa_xmit;
	int i, pack_len, sa_xmit_len, probes_sent, error;
	struct timeval to;
	struct ntptime udp_arrival_ntp;

	if (debug) MODULES_LOG_DEBUG("Listening...\n");

	probes_sent=0;
	sa_xmit_len=sizeof(sa_xmit);
	to.tv_sec=0;
	to.tv_usec=0;
	for (;;) 
	{
		FD_ZERO(&fds);
		FD_SET(usd,&fds);
		i=select(usd+1,&fds,NULL,NULL,&to);  /* Wait on read or error */
		if ((i!=1)||(!FD_ISSET(usd,&fds))) 
		{
			if (i==EINTR) 
			{
				continue;

			}
			if (i<0) perror("select");
			if (to.tv_sec == 0) {
				if (probes_sent >= num_probes &&
						num_probes != 0)
				{
					break;
				}

				send_packet(usd);
				++probes_sent;
				to.tv_sec=interval;
				to.tv_usec=0;
			}	

			continue;
		}
		pack_len=recvfrom(usd,incoming,sizeof_incoming,0,
				&sa_xmit,(socklen_t*)&sa_xmit_len);
		error = goodness+1;
		if (pack_len<0) 
		{
			perror("recvfrom");

		} 
		else if (pack_len>0 && (unsigned)pack_len<sizeof_incoming)
		{
			get_packet_timestamp(usd, &udp_arrival_ntp);
			check_source(pack_len, &sa_xmit, sa_xmit_len);
			error = rfc1305print(incoming_word, &udp_arrival_ntp, tv);
			/* udp_handle(usd,incoming,pack_len,&sa_xmit,sa_xmit_len); */
			return 0;

		} 
		else 
		{
			MODULES_LOG_DEBUG("Ooops.  pack_len=%d\n",pack_len);
			fflush(stdout);
		}
		if ( error < goodness && goodness != 0) 
			break;
		if (probes_sent >= num_probes && num_probes != 0) 
		{

			break;
		}
	}

	return -1;
}

static  bool ntp_time_get(char* hostname, int timeout, struct timeval *tv)
{
	int usd;  /* socket */
	bool ret = false;
  	/* Startup sequence */
	if ((usd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1)
		{perror ("socket"); return false;}

	if(setup_receive(usd, INADDR_ANY, 0) < 0)
	  {
		close(usd);
		MODULES_LOG_DEBUG("setup_receive failed!");
		return false;
	  }

	if(setup_transmit(usd, hostname, NTP_PORT) < 0)
	  {
		close(usd);
		MODULES_LOG_DEBUG("setup_transmit failed!");
		return false;
	  }

	ret = primary_loop(usd, 1, timeout, 0, tv) < 0? false: true;

	close(usd);

	return ret;
}

/* 获取ntp连接状态 */
bool sw_ntpclient_get_connstatus()
{
	
	return m_b_current_status;
	
}


void sw_ntpclient_reset_connstatus( )
{
	m_b_current_status=false;
}
	
bool sw_ntpclient_get_ntptime(char* hostname, struct timeval *tv)
{
	return ntp_time_get( hostname, 10, tv );
}

/**
 * 设置 TZ 环境变量
 * @param tz_minuteswest 以分钟表示的时区值。
 * @note 注意参数是 minutes west，即从0时区往西的时区为正，往东为负。如：中国为东八区，参数应传入 -8*60
 */
static void set_timezone_env( int tz_minuteswest )
{
	char tz_buf[12];
	char cmd[16];
	return 0;

	// MinuteToTimezone. (A little different from last function)
	short hour, min;
	char *p;

	memset( tz_buf, 0, sizeof(tz_buf) );
	p = tz_buf;
	if( tz_minuteswest<0 )
	{
		*p = '-';
		p++;
		tz_minuteswest = -tz_minuteswest;
	}
	else if( tz_minuteswest > 0 )
	{
		*p = '+';
		p++;
	}
	hour = tz_minuteswest/60;
	min = tz_minuteswest%60;
	if( min )
		snprintf( p, sizeof(tz_buf)-(p-tz_buf), "%d:%d", hour, min );
	else
		snprintf( p, sizeof(tz_buf)-(p-tz_buf), "%d", hour );

	memset( cmd, 0, sizeof(cmd) );
	snprintf( cmd, sizeof(cmd), "GMT%s", tz_buf );

	tzset();
	setenv( "TZ", cmd, 1 );

	printf( "[%s, L%d] set evn: %s\n", __FUNCTION__, __LINE__, cmd );
}

static bool auto_adust_time( unsigned long wparam, unsigned long lparam )
{
	time_t utc;	 
	time(&utc);
	if( !sumtime.b_sumtime &&  (utc >  (time_t)sumtime.start_time ) && (utc < (time_t)sumtime.end_time ) )
	{/* 设置夏令时区 */
		set_timezone_env(-(sumtime.time_zone));
		m_sumt_thread = NULL;
		sumtime.b_sumtime = true;
		ntpclient_send_dst_change(SW_NTP_INTO_DST, sumtime.start_time, sumtime.end_time);
		return false;
	}
	else if (sumtime.b_sumtime &&  utc >  (time_t)sumtime.end_time )
	{
		set_timezone_env(-m_n_time_zone);
		m_sumt_thread = NULL;
		sumtime.b_sumtime = false;
		ntpclient_send_dst_change(SW_NTP_EXIT_DST, sumtime.start_time, sumtime.end_time);
		return false;
	}
	else if ( sumtime.flag == 0 )
	{
		m_sumt_thread = NULL;
		sumtime.b_sumtime = false;
		return false;
	}
	sw_thrd_delay(500);
	return true;
}

static bool sumtime_adjust(void)
{
	if ( m_sumt_thread == NULL )
	{
		m_sumt_thread = sw_thrd_open("tAutoAdustTime", 200, 0,4096,(threadhandler_t)auto_adust_time,0,0);
		if(m_sumt_thread)
		{ 	
			sw_thrd_resume(m_sumt_thread);
		}
		else
		{/* 是否需要强制启动?? */
			MODULES_LOG_DEBUG("Can not create thread to adjust summer time\n");
			time_t utc;
			int i = 10 ;
			while (i-- > 0)
			{
				time(&utc);
				if( !sumtime.b_sumtime &&  (utc >  (time_t)sumtime.start_time ) && (utc < (time_t)sumtime.end_time ) )
				{
					set_timezone_env(-(sumtime.time_zone));
					sumtime.b_sumtime = true;
					ntpclient_send_dst_change(SW_NTP_INTO_DST, sumtime.start_time, sumtime.end_time);
					return true;
				}
				else if (sumtime.b_sumtime &&  utc >  (time_t)sumtime.end_time )
				{
					set_timezone_env(-m_n_time_zone);
					sumtime.b_sumtime = false;
					ntpclient_send_dst_change(SW_NTP_EXIT_DST, sumtime.start_time, sumtime.end_time);
					return true;
				}
				sw_thrd_delay(1000);
			}
		}
	}
	return true;
}
static void ntpclient_send_dst_change(sw_ntpstate_t state, uint32_t start_time, uint32_t end_time)
{
    if( m_event_post )
    {
		sw_event_t event;
		event.type = SW_NTP_EVENT;
		event.ntp.state = state;
		event.ntp.start_time = start_time;
		event.ntp.end_time = end_time;
        m_event_post( m_event_handler, &event);
    }
}
#ifdef SUPPORT_TTNET
static void sw_turkey_time_correction()
{
	time_t timep;
	struct tm *p;
	time(&timep);
	p=localtime(&timep); /*取得当地时间*/
	MODULES_LOG_DEBUG("[%d]%d %d %d %d %d %d\n",__LINE__, (1900+p->tm_year),( 1+p->tm_mon), p->tm_mday, p->tm_hour,p->tm_min,p->tm_sec);
	if(sw_turkey_summer_time())
	{
		int local_summer_zone = sw_timezone2minute("3");
		MODULES_LOG_DEBUG("local summer zone = %d, Current zome = %d\n",local_summer_zone,m_n_time_zone);
		if(local_summer_zone != m_n_time_zone)
		{
			sw_ntpclient_set_timezone(local_summer_zone);
			sw_ntpclient_set_localtime((1900+p->tm_year),(1+p->tm_mon), p->tm_mday, p->tm_hour+1, p->tm_min, p->tm_sec );
			time_t timep1;
			struct tm *p1;
			time(&timep1);
			p1=localtime(&timep1); /*取得当地时间*/
			MODULES_LOG_DEBUG("[%d]%d %d %d %d %d %d\n",__LINE__, (1900+p1->tm_year),( 1+p1->tm_mon), p1->tm_mday, p1->tm_hour,p1->tm_min,p1->tm_sec);
			sw_parameter_set("timezone","3");
			sw_parameter_save();
		}
	}
	else
	{
		int local_normal_zone = sw_timezone2minute("2");
		MODULES_LOG_DEBUG("local normal zone = %d, Current zome = %d\n",local_normal_zone,m_n_time_zone);
		if(local_normal_zone != m_n_time_zone)
		{
			sw_ntpclient_set_timezone(local_normal_zone);
			sw_ntpclient_set_localtime((1900+p->tm_year),(1+p->tm_mon), p->tm_mday, p->tm_hour-1, p->tm_min, p->tm_sec );
			sw_parameter_set("timezone","2");
			sw_parameter_save();
		}
	}
	return;
}

static bool sw_turkey_summer_time()
{
	swsyscmd("date > /ttnet/Turkey_zone");
	struct stat stat_info;
	char *ttnet_buf = NULL;
	if(lstat( "/ttnet/Turkey_zone",  &stat_info ) == 0)
	{
		FILE * fp;
		int ret = -1;
		fp = fopen( "/ttnet/Turkey_zone", "a+" );
		if(fp ==NULL)
		{
			printf("open failed!!\n");	
			return false;
		}
		fseek(fp,0,SEEK_END);
		int len = ftell(fp);
		fseek(fp,0,SEEK_SET);
		printf("read length = %d\n",len);
		if(len < 0)
		{
			fclose(fp);
			fp = NULL;
			printf("is NULL!\n");
			return false;
		}
		ttnet_buf = (char*)malloc(len);
		if(ttnet_buf == NULL)
		{
			fclose(fp);
			fp = NULL;
			printf("malloc failed!\n");
			return false;
		}
		memset(ttnet_buf,0,len);
		ret = fread(ttnet_buf,len,1,fp);
		if(ret==-1)
		{
			fclose(fp);
			fp = NULL;
			free(ttnet_buf);
			ttnet_buf = NULL;
			printf("read string is NULL\n");
			return false; 
		}
		fclose(fp);
		fp = NULL;
		swsyscmd("rm -rf /ttnet/Turkey_zone");
		if(ttnet_buf)
		{
			printf("buff is %s\n",ttnet_buf);
			if(strstr(ttnet_buf, "EET")) //正常非夏令时
			{
				free(ttnet_buf);
				ttnet_buf = NULL;
				return false;
			}
			else if(strstr(ttnet_buf, "EEST")) //土耳其夏令时时间
			{
				free(ttnet_buf);
				ttnet_buf = NULL;
				return true;
			}
		}
	}
	free(ttnet_buf);
	ttnet_buf = NULL;
	return false;
}

#endif

