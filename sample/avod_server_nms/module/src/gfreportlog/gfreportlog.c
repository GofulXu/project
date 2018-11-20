#include "gfapi.h"
#include "gflog.h"
#include "gfthrd.h"
#include "gfmutex.h"
#include "gftime.h"
#include "gfreportlog.h"
#include "gfparamerter.h"

#define SAVE_BUF_SIZE (512*1024)

#define LOG_PATH "./logs"

static FILE *m_local_fp = NULL;

static HANDLE m_mutex = NULL;

static char m_log_path[256];

static bool m_exit = false;

typedef struct _module_info
{
    int index;
    char name[32];
}module_info_t;

static HANDLE m_delete_thrd = NULL;

static module_info_t m_module_list[] =
{
    { APP_MODULE, "MAIN_APP_MODULE"},
    { DISTRIBUTION_MODULE, "DISTRIBUTION_MODULE"},
    { ADVERTISEMENT_MODULE, "ADVERTISEMENT_MODULE"},
    { SERIAL_PORT_MODULE, "SERIALPORT_MODULE"},
    { PERIPHERAL_MODULE, "PERIPHERAL_MODULE"},
    { REPORTLOG_MODULE, "REPORTLOG_MODULE"},
    { USB_MODULE, "USB_MODULE"},
    { MANAGER_MODULE, "MANAGER_MODULE"},
    { STREAM_MODULE, "STREAM_MODULE"},
    { HEARTCLIENT_MODULE, "MASTER PLAYCTRL"},
    { HEARTSERVER_MODULE, "SLAVE_PLAYCTRL"},
    { STREAMTRANSFER_MODULE, "STREAMTRANSFER_MODULE"},
    { BROADCAST_MODULE, "BROADCAST_MODULE"},
    { HTTP_MODULE, "HTTP_MODULE"},
    { COMMUNICATION_MODULE, "COMMUNICATION_MODULE"},
    { NETWORK_PROTOCOL_MODULE, "NETWORK_PROTOCOL_MODULE"},
    { BOWCLIENT_MODULE, "BOWCLIENT_MODULE"},
    { SERVERDETECT_MODULE, "SERVERDETECT_MODULE"},
    { PLAYLIST_MODULE, "PLAYLIST_MODULE"},
    { NTPCLIENT_MODULE, "NTPCLIENT_MODULE"},
    { DOWNLOAD_MODULE, "DOWNLOAD_MODULE"},
    { MEDIAFTPSYNC_MODULE, "MEIDAFTPSYNC_MODULE"},
    { ENCODER_MODULE, "ENCODER_MODULE"},
    { PLAYER_MODULE, "PLAYER_MODULE"},
    { PLUGIN_MODULE, "PLUGIN_MODULE"},
    { OPERATE_MODULE, "OPERATE_MODULE"}
};

static bool deleteLogFileProc(uint64_t lparam, uint64_t uparam)
{
    char log_path[128];
    DIR *dir = NULL;
    struct dirent *entry;
    struct tm special_day;
    int year = 0,month = 0,day = 0;
    time_t  previous_day,now;
    char delete_file[256];
    int delete_days = 10;
    
    if(m_exit)
        return false;

    gf_thrd_delay(120*1000);

    delete_days = gf_paramerter_get_int("save_days");
    if(delete_days < 1 || delete_days > 3*30)
        delete_days = 30;

    memset(log_path, 0, sizeof(log_path));
    snprintf(log_path, sizeof(log_path) - 1, "%s", LOG_PATH);
    
    if((dir = opendir( log_path )) == NULL)
    {
        GFREPORTLOG_ERROR("open %s error\n", log_path);
        gf_thrd_delay(2000);
        return true;
    }

    while((entry = readdir(dir)) != NULL)
    {
        if(!strcmp(entry->d_name,".") || !strcmp(entry->d_name, ".."))
        {
            continue;
        }

        memset(delete_file, 0, sizeof(delete_file));
        snprintf(delete_file, sizeof(delete_file) - 1, "%s/%s", LOG_PATH, entry->d_name);
        if(strcmp(delete_file, m_log_path) == 0)
            continue;

        memset(&special_day, 0, sizeof(struct tm));
        sscanf(entry->d_name, "%*[^_]_%4d%2d%2d", &year, &month, &day);
        special_day.tm_year=year-1900;special_day.tm_mon = month - 1;special_day.tm_mday = day;
        previous_day = mktime(&special_day);

        now = gf_time_get_time();
        //now = time(NULL);
        if(now - delete_days*86400 > previous_day)
        {
            unlink(delete_file);
            gf_reportlog_save(REPORTLOG_MODULE, "Delete log file %s !", entry->d_name);
        }
    }

    closedir(dir);
    gf_thrd_delay(120*1000);
    return true;
}

int gf_reportlog_init()
{
	m_mutex=gf_mutex_create();	

    m_delete_thrd=gf_thrd_open("deleteLogProc",100,0,100,deleteLogFileProc, NULL,0,0);
    if(m_delete_thrd)
    {
        gf_thrd_resume(m_delete_thrd);
    }
   	return 0;
}
void gf_reportlog_exit()
{
    m_exit = true;

    if(m_mutex)
    {
        gf_mutex_destroy( m_mutex );
        m_mutex = NULL;
    }
    if(m_local_fp)
    {
        fclose(m_local_fp);
        m_local_fp = NULL;
    }
    if(m_delete_thrd)
    {
        gf_thrd_close(m_delete_thrd, 0);
        m_delete_thrd = NULL;
    }
    return ;
}
#define LOG_BUF_SIZE 4096
static char save_buf[LOG_BUF_SIZE];
int gf_reportlog_save(int type, char *format, ...)
{
    time_t now;
    struct tm *local;
    char *module_info = NULL;
    char current_date[64];
    int i;
	va_list args;
 
    if(m_exit)
        return -1;

    memset(save_buf, 0, sizeof(save_buf)); 
	va_start( args, format );
	vsnprintf(save_buf,LOG_BUF_SIZE, format, args );
	va_end( args );

    now = gf_time_get_time();
	local=localtime(&now);

    memset(current_date, 0, sizeof(current_date));
    snprintf(current_date, sizeof(current_date)-1, "%04d%02d%02d", local->tm_year+1900,local->tm_mon+1,local->tm_mday);
    //judge today pass or not 
    if(m_local_fp)
    {
       if( !strstr(m_log_path, current_date) )
       {
            GFREPORTLOG_INFO("a new day begin!\n");
            fclose(m_local_fp);
            m_local_fp = NULL;
       }
    }

    if(m_mutex)
        gf_mutex_lock(m_mutex);

    for(i = 0; i < sizeof(m_module_list)/sizeof(module_info_t); i ++)
    {
        if(type == m_module_list[i].index)
        {
            module_info = m_module_list[i].name;
            break;
        }
    }
    {
        if(m_local_fp)
        {
            fprintf(m_local_fp,"[%04d-%02d-%02d_%02d:%02d:%02d][%s][%s]\n\r",local->tm_year+1900,local->tm_mon+1,local->tm_mday,local->tm_hour,local->tm_min,local->tm_sec,module_info,save_buf);
            fflush(m_local_fp);
        }
        else
        {
            char mac[64];
            i = 0;
            memset(mac, 0, sizeof(mac));

            if(0 > gf_paramerter_get("mac", mac, sizeof(mac)))
            {
                snprintf(mac, sizeof(mac)-1,"%s", "00:07:63:11:22:33");
            }

            while(i < strlen(mac))
            {
                if(mac[i] == ':')
                    mac[i] = '-';
                i ++;
            }

            snprintf(m_log_path, sizeof(m_log_path)-1, "%s", LOG_PATH);

            if( -1 == access(m_log_path, F_OK|R_OK|W_OK) )
            {
                if(-1 == mkdir( m_log_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
                {
                    GFREPORTLOG_ERROR("[%s:%d] make plandir %s failed!!!\n",__FUNCTION__,__LINE__,m_log_path);
                    if(m_mutex)
                        gf_mutex_unlock(m_mutex);
                    return -1;
                }
            }
            snprintf(m_log_path+strlen(m_log_path), sizeof(m_log_path)-strlen(m_log_path),"/%s_%04d%02d%02d.txt",mac,local->tm_year+1900,local->tm_mon+1,local->tm_mday);

            if( (m_local_fp = fopen(m_log_path,"a+")) == NULL )
            {
                GFREPORTLOG_ERROR("open %s error\n",m_log_path);
                if(m_mutex)
                    gf_mutex_unlock(m_mutex);
                return -1;
            }
            {
                fprintf(m_local_fp,"[%04d-%02d-%02d_%02d:%02d:%02d][%s][%s]\n\r",local->tm_year+1900,local->tm_mon+1,local->tm_mday,local->tm_hour,local->tm_min,local->tm_sec,module_info,save_buf);
                fflush(m_local_fp);
            }
        }
    }
#ifdef SUPPORT_WRITETODISK
	sync();
#endif
    if(m_mutex)
        gf_mutex_unlock(m_mutex);
    return 0;
}

int gf_reportlog_save02hex(int type, char *warn_text, char *data,int size)
{
	if(!data)
		return -2;
   int i = 0;
   char tbuf[4];
   char log_buf[LOG_BUF_SIZE];
   memset(log_buf, 0, sizeof(log_buf));
   for(i = 0; i < size; i++)
   {
	   memset(tbuf,0,sizeof(tbuf));
       sprintf(tbuf," %02x", data[i]);
	   strcat(log_buf, tbuf);
   }
   return gf_reportlog_save(type, "%s%s", warn_text, log_buf);
}
