#include "gfapi.h"
#include "gflog.h"
#include "gfthrd.h"
#include "gfmutex.h"
#include "gfreportlog.h"
#include "gfparamerter.h"
#include "gftime.h"

static HANDLE m_mutex = NULL;

static bool m_exit = false;

static HANDLE m_timedetect_thrd = NULL;

static char m_time_reboot[128];

static int m_hour;

static int m_minute;

static int m_reboot_status = 0;

static bool timeDetectProc(uint32_t lparam, uint32_t uparam)
{
    time_t now;
    struct tm *local;
    if(m_exit)
        return false;

    now = gf_time_get_time();
	local=localtime(&now);
  
	//5分钟后参数复位，防止第二个周天板卡无法重启
	if(m_reboot_status && local->tm_hour == m_hour && local->tm_min >= m_minute + 5)
	{
		gf_paramerter_set_int("reboot_status", 0);
		gf_paramerter_save();
	}
	
	m_reboot_status = gf_paramerter_get_int("reboot_status");
	if(m_reboot_status <= 0)
		m_reboot_status = 0;
	if(!m_reboot_status && local->tm_hour == m_hour && (local->tm_min >= m_minute && local->tm_min <= m_minute + 2))
	{
		gf_reportlog_save(SERIAL_PORT_MODULE, "%s", "one day is gone,it will be reboot");
		
		gf_paramerter_set_int("reboot_status", 1);
		gf_paramerter_save();
		
		sync();

		if(gf_paramerter_get_int("board_status") == 1)
		{
			gf_thrd_delay(5000);
			//停止喂狗
			gf_watchdog_stop_feeddog();
		}
		else
		{
			//滚动退出，停止喂狗
			gf_scrollmsg_exit();
		}
		//gf_event_t event;
		//event.system.type = gf_SYSTEM_EVENT;
		//event.system.action = gf_EVT_REBOOT;
		//app = gf_app_get_context();
		//gf_evtdispatcher_on_event(app->event_dispatcher, &event);
	}
	gf_thrd_delay(30*1000);
    return true;
}

int gf_timedetect_init()
{
	m_mutex=gf_mutex_create();	
    memset(m_time_reboot, 0, sizeof(m_time_reboot));

    if(!gf_paramerter_get("time_reboot",m_time_reboot,sizeof(m_time_reboot)))
        snprintf(m_time_reboot, sizeof(m_time_reboot)-1, "%s", "01:00");
	gf_reportlog_save(SERIAL_PORT_MODULE, "m_reboot_time=%s",m_time_reboot);
    
    sscanf(m_time_reboot, "%d:%d",&m_hour, &m_minute);
	gf_reportlog_save(SERIAL_PORT_MODULE, "the reboot time is %d:%d",m_hour,m_minute);

	m_timedetect_thrd=gf_thrd_open("timeDetect",100,0,100,timeDetectProc,0,0);
    if(m_timedetect_thrd)
    {
        gf_thrd_resume(m_timedetect_thrd);
    }
   	return 0;
}
int gf_timedetect_exit()
{
    m_exit = true;

    if(m_mutex)
    {
        gf_mutex_destroy( m_mutex );
        m_mutex = NULL;
    }
    if(m_timedetect_thrd)
    {
        gf_thrd_close(m_timedetect_thrd, 0);
        m_timedetect_thrd = NULL;
    }
    return 0;
}
