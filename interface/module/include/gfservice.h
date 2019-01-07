#ifndef __GFSERVICE_H__
#define __GFSERVICE_H__

#ifdef __cplusplus
extern "C"{
#endif

#define GF_SERVICE_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "swservice", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GF_SERVICE_LOG_INFO( format, ...) 	gf_log(LOG_LEVEL_INFO, "swservice", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GF_SERVICE_LOG_WARN( format, ...) 	gf_log(LOG_LEVEL_WARN, "swservice", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GF_SERVICE_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "swservice", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GF_SERVICE_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "swservice", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
	
int gf_service_init();
int gf_service_exit();
bool gf_service_check();
int gf_sendto_service(char *send_data, int size, char *recv_buf, int recv_size);
#endif
