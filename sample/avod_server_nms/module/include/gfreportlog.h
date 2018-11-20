/* 
  Function: recv data from mediatest
  Author  : test
  Date    : 2013.06.28
*/

#ifndef __REPORTLOG_H__
#define __REPORTLOG_H__

#ifdef __cplusplus
extern "C"  {
#endif

#define APP_MODULE              1000
#define DISTRIBUTION_MODULE     1001
#define ADVERTISEMENT_MODULE    1002
#define SERIAL_PORT_MODULE      1003
#define PERIPHERAL_MODULE       1004
#define REPORTLOG_MODULE        1005
#define STREAM_MODULE           1006
#define USB_MODULE              1007
#define MANAGER_MODULE          1008
#define HEARTCLIENT_MODULE      1009
#define HEARTSERVER_MODULE      1010
#define STREAMTRANSFER_MODULE   1011
#define BROADCAST_MODULE        1012
#define COMMUNICATION_MODULE    1013
#define HTTP_MODULE    			1014
#define NETWORK_PROTOCOL_MODULE 1015
#define SERVERDETECT_MODULE 	1016
#define BOWCLIENT_MODULE 		1017
#define PLAYLIST_MODULE 		1018
#define NTPCLIENT_MODULE 		1019
#define DOWNLOAD_MODULE 		1020
#define MEDIAFTPSYNC_MODULE		1021
#define ENCODER_MODULE			1022
#define OPERATE_MODULE			1023

#define PLAYER_MODULE 			1050
#define PLUGIN_MODULE 			1051

#define GFREPORTLOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "gfreportlog", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFREPORTLOG_INFO( format, ...) 	gf_log(LOG_LEVEL_INFO, "gfreportlog", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFREPORTLOG_WARN( format, ...) 	gf_log(LOG_LEVEL_WARN, "gfreportlog", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFREPORTLOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "gfreportlog", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFREPORTLOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "gfreportlog", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)


int gf_reportlog_init();

void gf_reportlog_exit();

int gf_reportlog_save(int type, char *format, ...);

int gf_reportlog_save02hex(int type, char *warn_text, char *data,int size);
#ifdef __cplusplus
}
#endif

#endif /*__REPORTLOG_H__*/


