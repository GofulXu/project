#ifndef __GFTFTPCLIENT_H_
#define __GFTFTPCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif


#define GFTFTPCLIENT_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "tftpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFTFTPCLIENT_LOG_INFO( format, ...)	 	gf_log(LOG_LEVEL_INFO, "tftpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__) 
#define GFTFTPCLIENT_LOG_WARN( format, ...)		gf_log(LOG_LEVEL_WARN, "tftpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__) 
#define GFTFTPCLIENT_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "tftpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFTFTPCLIENT_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "tftpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

#define REMOTE_tftp_URL "192.168.1.242"

int gf_tftp_downloadfile(char *ip, char *savepath, char *filename, int timeout);

int gf_tftp_uploadfile(char *ip, char *filepath, int timeout);

#ifdef __cplusplus
}
#endif

#endif
