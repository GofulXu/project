#ifndef __GFFTPCLIENT_H_
#define __GFFTPCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif


#define GFFTPCLIENT_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "ftpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFFTPCLIENT_LOG_INFO( format, ...)	 	gf_log(LOG_LEVEL_INFO, "ftpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__) 
#define GFFTPCLIENT_LOG_WARN( format, ...)		gf_log(LOG_LEVEL_WARN, "ftpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__) 
#define GFFTPCLIENT_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "ftpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFFTPCLIENT_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "ftpclient", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

#define REMOTE_FTP_URL "ftp://192.168.0.108/"

char *gf_ftp_download_to_mem(const char *ftpurl, long *msize, const char *username, const char *password, const int timeout);

int gf_ftp_download_to_file(const char *ftpurl, const char *local_name, const char *username, const char *password, const int timeout);

int gf_ftp_upload_from_file(const char *ftpurl, const char *local_name, const char *username, const char *password, const int timeout);


int gf_ftp_upload_from_mem(const char *ftpurl, const char *membuf, unsigned long size, const char *username, const char *password, const int timeout);

#ifdef __cplusplus
}
#endif

#endif
