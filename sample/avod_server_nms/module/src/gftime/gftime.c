#include "gfapi.h"
#include "gfthrd.h"
#include "gfmutex.h"
#include "gflog.h"
#include "gftime.h"

static time_t m_adjust_time = 0; 
static unsigned long m_adjust_tick = 0;

static HANDLE m_time_mutex = NULL;

static char m_current_strtime[128];

int gf_time_init()
{
    m_adjust_time = time(NULL);
    m_adjust_tick = gf_thrd_get_tick();

    m_time_mutex = gf_mutex_create();
   return 0;
}

int gf_time_exit()
{
	if(m_time_mutex)
		gf_mutex_destroy(m_time_mutex);

	return 0;
}

int gf_time_get_time()
{
    return m_adjust_time + (gf_thrd_get_tick() - m_adjust_tick)/1000;
}


char* gf_time_get_strtime()
{
    gf_mutex_lock(m_time_mutex);

    memset(m_current_strtime, 0, sizeof(m_current_strtime));

    time_t current_time;
    struct tm *local;
    current_time = m_adjust_time + (gf_thrd_get_tick() - m_adjust_tick)/1000;
    local = localtime(&current_time);

    snprintf(m_current_strtime, sizeof(m_current_strtime), "%04d-%02d-%02d %02d:%02d:%02d", 
             local->tm_year + 1900,
             local->tm_mon + 1,
             local->tm_mday,
             local->tm_hour,
             local->tm_min,
             local->tm_sec);

    gf_mutex_unlock(m_time_mutex);
    return m_current_strtime;
}

int gf_time_adjust_strtime(const char *strtime)
{
    struct tm adjust_tm;
    int year=0,month=0,day=0,hour=0,min=0,sec=0;
    if(!strstr(strtime, "-") || !strstr(strtime, ":"))
        return -1;
    GFTIME_LOG_DEBUG("TIme = *%s*\n",strtime);
    sscanf(strtime, "%d-%d-%d %d:%d:%d", &year, &month,&day,&hour,&min,&sec);
    GFTIME_LOG_DEBUG("[%s][%d] year=%d,month=%d,day=%d,hour=%d,min=%d,sec=%d\n",__FUNCTION__,__LINE__,year,month,day,hour,min,sec);
    
    adjust_tm.tm_year = year - 1900;
    adjust_tm.tm_mon = month - 1;
    adjust_tm.tm_mday = day;
    adjust_tm.tm_hour = hour;
    adjust_tm.tm_min = min;
    adjust_tm.tm_sec = sec;

    m_adjust_time = mktime(&adjust_tm);
    m_adjust_tick = gf_thrd_get_tick();
    return 0;
}
int gf_time_adjust_time(int year, int mon, int day, int hour, int min, int sec)
{
    struct tm adjust_tm;
    memset(&adjust_tm, 0, sizeof(struct tm));
    adjust_tm.tm_year = year - 1900;
    adjust_tm.tm_mon = mon - 1;
    adjust_tm.tm_mday = day;
    adjust_tm.tm_hour = hour;
    adjust_tm.tm_min = min;
    adjust_tm.tm_sec = sec;

    m_adjust_time = mktime(&adjust_tm);
    m_adjust_tick = gf_thrd_get_tick();
    return 0;
}
