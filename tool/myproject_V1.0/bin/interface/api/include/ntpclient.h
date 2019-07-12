#ifndef __NTFCLIENT_H__
#define __NTPCLIENT_H__

#define GFNTPCLIENT_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "ntpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFNTPCLIENT_LOG_INFO( format, ...)	 	gf_log(LOG_LEVEL_INFO, "ntpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__) 
#define GFNTPCLIENT_LOG_WARN( format, ...)		gf_log(LOG_LEVEL_WARN, "ntpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__) 
#define GFNTPCLIENT_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "ntpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFNTPCLIENT_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "ntpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

#define CHINA_NTP_SERVER1	"s1a.time.edu.cn"
#define CHINA_NTP_SERVER2	"ntp.sjtu.edu.cn"
#define CHINA_NTP_SERVER3	"s2b.time.edu.cn"

#define NTP_PORT (123)
void setup_receive(int usd, unsigned int interface, short port);


void setup_transmit(int usd, char *host, short port);


int primary_loop(int usd, int num_probes, int interval, int goodness, struct timeval *tv);

long get_time_from_ntpserver(const char *serveraddr);
#endif /*__NTPCLIENT_H__*/
