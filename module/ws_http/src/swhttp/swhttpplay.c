/*
 * =====================================================================================
 *       Filename:  swhttpplay.cpp
 *    Description:  
 *        Version:  1.0
 *        Created:  2011年09月09日 09时40分01秒
 *
 *         Author:  wanghuan, 
 *        Company:  sunniwell.net
 * =====================================================================================
 */

#include <sys/types.h>
#include <dirent.h>

#include "swapi_linux.h"
#include "swthrd.h"
#include "md5.h"
#include "swhttpserver.h"
#include "swhttpclient.h"
#include "swhttpplay.h"
#include "swlog.h"

typedef enum
{
	STBCHECK,
	STBMODEEXTERN,
	STBLISTVERSION,
	STBMODESTOP,
	STBMODERM,
	STBLIST,
	STBMEDIAPLAY,
	STBMEDIASTOP,
	STBMEDIAPAUSE,
	STBMEDIARESUME,
	STBMEDIASEEK,
	STBMEDIAINFO,
	STBVOLUMEADD,
	STBVOLUMEDEC,
	STBVOLUMEGET,
	STBVOLUMESET,
	STBIMAGESHOWFULL,
	STBIMAGESHOWWINDOW,
	STBIMAGEEFFECT,
	STBIMAGESTOP,
	STBBACKGROUND
}STB_ORDER;

typedef enum httpplay_statu
{
	HTTPPLAY_NULL,
	HTTPPLAY_IMAGE,
	HTTPPLAY_VIDEO,
	HTTPPLAY_AUDIO,
	HTTPPLAY_FREE
}HTTPPLAYSTATU;

typedef struct playinfo_t
{
	int  type;
	char filename[128];
	char playname[128];
	char country[32];
	struct playinfo_t *next;
}PLAYINFO;


/** 
 * @brief 生成字符串的MD5值
 * @param filebuf 字符串；size 字符串长度；dkey 生成的MD5值
 */
static void check_md5(char *dkey, char *filebuf, int size)
{
	struct SWMD5Context md5ctx;
	char buf[128];
	memset(buf,0,sizeof(buf));
	unsigned char key[16];

	memset(key,0,sizeof(key));
	SWMD5Init(&md5ctx);
	SWMD5Update(&md5ctx,(unsigned char const*)filebuf, size);
	SWMD5Final(key,&md5ctx);

	int i = 0;
	for(i = 0;i < 16; i++)
	{
		sprintf(dkey+i*2, "%02x", key[i]);
	}

	sw_log_debug("[%s %s %d] buf:%s md5:%s\n", __FUNCTION__, __FILE__, __LINE__, filebuf, dkey);
}

/** 
 * @brief   The size of file ,in bytes
 * @param   filename the name of file
 * @return  size of file; failure -1
 */
unsigned long get_filesize(const char *filename)  
{  
	if(NULL == filename)
	{
		return -1;
	}

    struct stat buf;  
    if(stat(filename, &buf) < 0)  
    {  
		perror("stat");
        return -1;  
	}

	printf("%s:%ld bytes\n", filename, buf.st_size);
	return buf.st_size;  
}

#define HTTP_SERVER_PORT 13008
#define HTTPPLAY_AUDIO_IMAGE "./media/audioimage.jpg"
#define HTTPPLAY_STB_IMAGE   "./media/stbimage.png"

static int httpplay_callback(SHttpConnectObj *obj, uint32_t lParam);
static bool httpplay_check_proc( unsigned long wparam, unsigned long lparam );

static char m_mac[32];
static PLAYINFO *httpplay_media_getfilename(char *playname);
static PLAYINFO *playinfo_head = NULL;
static HANDLE httpplay_check_thrd = NULL;
static HANDLE m_httpserver = NULL;
static int  m_STBlistVersion = 0;
static int  m_mediatype = 0;  //0 空闲， 1 单张图片， 2 多张图片, 3 视频, 4 音频
static char m_audioimage[256] = {0};
static char m_stbimage[256] = {0};
static int  httpplay_tick = 0;
static bool extern_mode = false;

/** 
 * @brief 图片退出
 */
static void httpplay_image_stop()
{
	sw_log_debug("[%s %s %d] m_mediatype=%d\n", __FUNCTION__, __FILE__, __LINE__, m_mediatype);
}

/** 
 * @brief swmedia 退出
 */
static void httpplay_media_stop()
{
	m_mediatype = 0;
}

static int httpplay_check_init()
{
	httpplay_tick = sw_thrd_get_tick();
	if(NULL == httpplay_check_thrd)
	{
		httpplay_check_thrd = sw_thrd_open("HttpPlayCheck", 80, 0, 1024*4, (PThreadHandler)httpplay_check_proc, 0, 0);

		if( NULL == httpplay_check_thrd )
		{
			sw_log_debug("[%s %s %d] httpplay check failed\n", __FUNCTION__, __FILE__, __LINE__);
			return -1;
		}
		else
		{
			sw_thrd_resume(httpplay_check_thrd);
		}
	}

	return 0;
}

static void httpplay_check_exit()
{
	if(NULL != httpplay_check_thrd)
	{
		sw_thrd_close(httpplay_check_thrd, 1000);
		httpplay_check_thrd = NULL;
	}
}

/** 
 * @brief  httpplay init, start httpserver and check thrd
 * @return succes, 0; failure, -1
 */
int httpplay_init() 
{
	if(strlen(sw_advertisement_get_diskpath()) > 0)
	{
		char path[256] = {0};
		snprintf(path, sizeof(path), "%s/program", "./media");
		sw_log_debug("[%s %s %d] PATH:%s\n", __FUNCTION__, __FILE__, __LINE__, path);
		if(access(path, F_OK) == 0)
		{
			httpplay_readdir(path);
		}
	}

	m_STBlistVersion = 0;
	m_httpserver = sw_httpserver_open(htons((unsigned short)HTTP_SERVER_PORT), httpplay_callback, 0);

	if(m_httpserver == NULL)
	{
		sw_log_debug("[%s %s %d] \n", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}
	else
	{
		sw_log_debug("[%s %s %d] \n", __FUNCTION__, __FILE__, __LINE__);
	}
 
	return 0;
}

/** 
 * @brief  httpplay exit
 */
void httpplay_exit()
{
	if(NULL != m_httpserver)
	{
		sw_httpserver_close(m_httpserver);
		m_httpserver = NULL;
	}

	if(NULL != playinfo_head)
	{
		PLAYINFO *playinfo = playinfo_head;
		playinfo_head = playinfo_head->next;
		free(playinfo);
	}

	sw_media_stop();
	httpplay_check_exit();
}

static void httpplay_video_mode()
{
}

static int httpplay_respond(SHttpConnectObj *obj, HttpResponseNum numcode)
{
	int timeout = 5000;
	HttpResponseNum response_num = numcode;

	if(sw_httpserver_send_response_header(obj, response_num, "media/json", NULL, "Keep-Alive", 0, timeout) < 0)
	{
		sw_log_debug("sw_httpserver_send_response_header failed!!!\n");
	}
	else
	{
		sw_log_debug("send header ok\n");
	}

	if(NULL != obj)
	{
		sw_httpserver_close_connectobj(obj);
	}

	return 0;
}

/** 
 * @brief 音频播放时显示背景
 */
bool httpplay_audio_showimage()
{

}

/** 
 * @brief STB空闲播放时显示背景
 */
bool httpplay_stb_showimage()
{

}

/** 
 * @brief
 * @return 成功,返回0;否则,返回-1
 */
bool httpplay_image_save(char *buf, SHttpConnectObj *obj)
{
   if( NULL == buf || NULL == obj)
   {
	   return false;
   }

   char *p = NULL;
   char m_audioimage[256] = {0};
   char m_stbimage[256] = {0};

   int len = obj->request_header.content_length;
   if((p = strstr(buf, "audio&")) != NULL)
   {
	   p += strlen("audio&");
	   char *tmp = p;
	   p = strstr(tmp, "&");
	   memset(m_audioimage, 0, sizeof(m_audioimage));
	   snprintf(m_audioimage, sizeof(m_audioimage), "%s", "./media");

	   strncpy(m_audioimage+strlen(m_audioimage), tmp, p-tmp);
	   sw_log_debug("[%s %s %d] %s\n", __FUNCTION__, __FILE__, __LINE__, m_audioimage);

	   FILE *fp = fopen(m_audioimage, "w");
	   if(fp)
	   {
		   if(p)
		   {	
			   ++p;
			   int size = fwrite(p, 1, len, fp);
			   sw_log_debug("[%s %s %d] %d\n", __FUNCTION__, __FILE__, __LINE__, size);
		   }
			fclose(fp);
	   }

	   return true;
   }
   else if((p = strstr(buf, "stbimage&")) != NULL)
   {
	   p += strlen("stbimage&");
	   char *tmp = p;
	   p = strstr(tmp, "&");
	   memset(m_stbimage, 0, sizeof(m_stbimage));
	   strncpy(m_stbimage, tmp, p-tmp-1);

	   FILE *fp = fopen(m_stbimage, "w");
	   if(fp)
	   {
		fwrite(buf, 1, len, fp);
		fclose(fp);
	   }
	   return true;
   }

   return false;
}

/** 
 * @brief  httpplay return STB mac and client md5 of IP
 * @return succes, 0; failure, -1
 */
int httpplay_checkstb_respond(SHttpConnectObj *obj)
{	
	struct sockaddr_in from;
	char client_ip[128];
	char md5_ip[128];
	char content[256];

	if(obj == NULL)
		return -1;
	memset(md5_ip,0,sizeof(md5_ip));
	memset(content,0,sizeof(content));
	memset(client_ip,0,sizeof(client_ip));

	from.sin_addr.s_addr = obj->from_ip;
	strcpy(client_ip,inet_ntoa( from.sin_addr));	
	printf("accept_ip:%s\n", client_ip);
	check_md5(md5_ip, client_ip, strlen(client_ip));

	sprintf(content,"mac=%s&md5_ip=%s\n\r", m_mac, md5_ip);
	sw_log_debug("[%s %s %d] content: %s\n", __FUNCTION__, __FILE__, __LINE__, content);

	int len = strlen(content);
	int timeout = 5000;
	HttpResponseNum response_num = HTTP_OK;

	if(sw_httpserver_send_response_header(obj, response_num, "text/json", NULL, "Keep-Alive", len, timeout) < 0)
	{
		sw_log_debug("sw_httpserver_send_response_header failed!!!\n");
	}
	else
	{
		sw_log_debug("send header ok\n");
	}

	if(sw_httpserver_send_response_content(obj, content, len, timeout	) < 0)
	{
		sw_log_debug("sw_httpserver_send_response_content failed!!!\n");
	}
	else
	{
		sw_log_debug("send content ok\n");
	}

	if(NULL != obj)
	{
		sw_httpserver_close_connectobj(obj);
	}

	return 0;
}

/** 
 * @brief get httpplay order
 * @param client send message , http content not include header
 * return order ;failure -1
 */
int httpplay_getorder(char *buf)
{
	if(NULL == buf)
	{
		sw_log_debug("[%s %s %d] BUF is NULL\n", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}

	if(strstr(buf, "KEY=STBcheck") != NULL)
	{
		return STBCHECK;
	}
	else if(strstr(buf, "STBmode=extern") != NULL)
	{
		return STBMODEEXTERN;
	}
	else if(strstr(buf, "STBlistVersion") != NULL)
	{
		return STBLISTVERSION;
	}
	else if(strstr(buf, "STBmode=stop") != NULL)
	{
		return STBMODESTOP;
	}
	else if(strstr(buf, "STBmode=rm") != NULL)
	{
		return STBMODERM;
	}
	else if(strstr(buf, "STBlist") != NULL)
	{
		return STBLIST;
	}
	else if(strstr(buf, "STBmedia=play") != NULL)
	{
		return STBMEDIAPLAY;
	}
	else if(strstr(buf, "STBmedia=pause") != NULL)
	{
		return STBMEDIAPAUSE;
	}
	else if(strstr(buf, "STBmedia=resume") != NULL)
	{
		return STBMEDIARESUME;
	}
	else if(strstr(buf, "STBmedia=seek") != NULL)
	{
		return STBMEDIASEEK;
	}
	else if(strstr(buf, "STBmedia=stop") != NULL)
	{
		return STBMEDIASTOP;
	}
	else if(strstr(buf, "STBmedia=mediainfo") != NULL)
	{
		return STBMEDIAINFO;
	}
	else if(strstr(buf, "STBmedia=addvolume") != NULL)
	{
		return 	STBVOLUMEADD;
	}
	else if(strstr(buf, "STBmedia=decvolume") != NULL)
	{
		return STBVOLUMEDEC;
	}
	else if(strstr(buf, "STBmedia=getvolume") != NULL)
	{
		return STBVOLUMEGET;
	}
	else if(strstr(buf, "STBmedia=setvolume") != NULL)
	{
		return STBVOLUMESET;
	}
	else if(strstr(buf, "STBimageshow=showfull:") != NULL)
	{
		return STBIMAGESHOWFULL;
	}
	else if(strstr(buf, "STBimageshow=showwindow:") != NULL)
	{
		return STBIMAGESHOWWINDOW;
	}
	else if(strstr(buf, "STBimageshow=imageeffect") != NULL)
	{
		return STBIMAGEEFFECT;
	}
	else if(strstr(buf, "STBimageshow=stop") != NULL)
	{
		return STBIMAGESTOP;
	}
	else if(strstr(buf, "STBbackground=") != NULL)
	{
		return STBBACKGROUND;
	}

	return -1;
}

/** 
 * @brief  check client send md5
 * @return 成功,返回0;否则,返回-1
 */
int httpplay_checkmd5(SHttpConnectObj *obj, char *buf)
{
	struct sockaddr_in from;
	char client_ip[128] = {0};
	char md5_ip[128] = {0};
	char buffer[256] = {0};
	char session_key[256] = {0};
	bool bMd5 = false;

	if(NULL == buf || NULL == obj)
	{
		return -1;
	}
	from.sin_addr.s_addr = obj->from_ip;
	strcpy(client_ip, inet_ntoa( from.sin_addr));	

	sw_log_debug("[%s %s %d] Client IP: %s\n", __FUNCTION__, __FILE__, __LINE__, client_ip);
	check_md5(md5_ip, client_ip, strlen(client_ip));

	sprintf(buffer,"%s%s", m_mac, md5_ip);
	sw_log_debug("[%s %s %d] Buffer:%s\n", __FUNCTION__, __FILE__, __LINE__, buffer);

	check_md5(session_key,buffer,strlen(buffer));
	printf("HTTPPLAY MD5:%s\n",session_key);
	char *p = strstr(buf, "KEY=");
	if(p != NULL)
	{
		if(strncmp(p+4, session_key,strlen(session_key))==0)
		{
			bMd5 = true;
			sw_log_debug("[%s %s %d] Check MD5 OK!\n", __FUNCTION__, __FILE__, __LINE__);

			return 0;
		}
	}

	/* send 401 response Unauthorized */
	char content[256] = {0};
	int timeout = 5000;
	HttpResponseNum  response_num = HTTP_UNAUTHORIZED;
	sw_log_debug("[%s %s %d] Check MD5 Failed!\n", __FUNCTION__, __FILE__, __LINE__);

	sprintf(content,"the correct md5 is %s\r\nthe mac is %s\r\nthe md5 ip is %s\r\n",session_key, m_mac, md5_ip);
	int len = strlen(content);
	if( sw_httpserver_send_response_header( obj, response_num, "text/html;charset=utf-8",
				NULL, "Keep-Alive", len, timeout ) <= 0 )
	{
		sw_log_debug("[%s] line %d\n", __FILE__, __LINE__);
	}

	if( sw_httpserver_send_response_content( obj, content, len, timeout ) < 0 )
	{
		sw_log_debug("[%s] line %d\n", __FILE__, __LINE__);
	}

	if(obj != NULL)
	{
		sw_httpserver_close_connectobj(obj);
	}

	return -1;
}

/** 
 * @brief show picture
 * @return 成功,返回 true;否则,返回 false
 */
bool httpplay_image_show(char *file)
{
	return true;
}

bool httpplay_image_showwindow(char *file)
{
	return true;
}

/** 
 * @brief get medialist include pictures ,videos, audios, medialist save in buffer
 * @return 成功,返回0;否则,返回-1
 */
int httpplay_medialist_get(char *buffer)
{
	if(NULL == playinfo_head || NULL == buffer)
	{
		sw_log_debug("[%s %s %d] Playinfo head or buffer is NULL \n", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}

	memset(buffer, 0, strlen(buffer));
	PLAYINFO *playinfo = playinfo_head;
	snprintf(buffer, strlen("vedioplaylist=")+1, "%s", "vedioplaylist=");

	while(NULL != playinfo)
	{
		if(playinfo->type == 2)
		{
			if( strlen(playinfo->country) != 0)
			{
				snprintf(buffer+strlen(buffer), sizeof(playinfo->playname)+sizeof(playinfo->country)+2, "/%s/%s&", playinfo->country, playinfo->playname);
			}
			else
			{
				snprintf(buffer+strlen(buffer), sizeof(playinfo->playname), "%s&", playinfo->playname);
			}
		}
		playinfo = playinfo->next;
	}

	playinfo = playinfo_head;
	if(buffer[strlen(buffer) - 1] == '&')
	{
		buffer[strlen(buffer) - 1] = '\0';
	}
	snprintf(buffer+strlen(buffer), strlen("||audioplaylist=")+1, "%s", "||audioplaylist=");
	while(NULL != playinfo)
	{
		if(playinfo->type == 3)
		{
			if( strlen(playinfo->country) != 0)
			{
				snprintf(buffer+strlen(buffer), sizeof(playinfo->playname)+sizeof(playinfo->country)+2, "/%s/%s&", playinfo->country, playinfo->playname);
			}
			else
			{
				snprintf(buffer+strlen(buffer), sizeof(playinfo->playname), "%s&", playinfo->playname);
			}
		}
		playinfo = playinfo->next;
	}

    playinfo = playinfo_head;
	if(buffer[strlen(buffer) - 1] == '&')
	{
		buffer[strlen(buffer) - 1] = '\0';
	}
	snprintf(buffer+strlen(buffer), strlen("||imageplaylist=")+1, "%s", "||imageplaylist=");
	while(NULL != playinfo)
	{
		if(playinfo->type == 1)
		{
			if( strlen(playinfo->country) != 0)
			{
				snprintf(buffer+strlen(buffer), sizeof(playinfo->playname)+sizeof(playinfo->country)+2, "/%s/%s&", playinfo->country, playinfo->playname);
			}
			else
			{
				snprintf(buffer+strlen(buffer), sizeof(playinfo->playname), "%s&", playinfo->playname);
			}

		}
		playinfo = playinfo->next;
	}

	if(buffer[strlen(buffer) - 1] == '&')
	{
		buffer[strlen(buffer) - 1] = '\0';
	}
	return 0;
}

static void httpplay_medialist_destroy()
{
	PLAYINFO *playinfo = NULL;
	
	while(NULL != playinfo_head)
	{
		playinfo = playinfo_head;
		playinfo_head = playinfo_head->next;
		free(playinfo);
	}
}

/** 
 * @brief
 */
static bool httpplay_check_proc( unsigned long wparam, unsigned long lparam )
{
	int tickdiff = sw_thrd_get_tick() - httpplay_tick;

	//sw_log_debug("[%s %s %d] %d %d\n", __FUNCTION__, __FILE__, __LINE__, m_mediatype, tickdiff);
	if(m_mediatype == 0 && tickdiff > 1000*30 && tickdiff < 1000*32)
	{
		m_mediatype = 5;
		sw_thrd_delay(1000*2);
	}
	else if(m_mediatype == 3 || m_mediatype == 4)
	{
		httpplay_tick = sw_thrd_get_tick();
		{
			m_mediatype = 0;
		}
	}
	else if(m_mediatype == 5 && tickdiff > 1000*60*3)
	{
		m_mediatype = 0;

		httpplay_check_thrd = NULL;
		extern_mode = false;
		return false;
	}
	else if(m_mediatype != 0 && m_mediatype != 5 )
	{
		httpplay_tick = sw_thrd_get_tick();
	}

	sw_thrd_delay(500);
	return true;
}

static int httpplay_get_respond(SHttpConnectObj *obj)
{
	if(NULL == obj)
	{
		return -1;
	}
    
	char file[128] = {0};
	int timeout = 5000;

	char *p = strcasestr(obj->request_header.request_url, "?KEY");
	if(NULL != p)
	{
		strncpy(file, obj->request_header.request_url+1, p - obj->request_header.request_url - 1);
	}

	if(httpplay_checkmd5(obj, obj->request_header.request_url) == -1)
	{
		sw_log_debug("[%s %s %d] Check MD5 failed\n", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	char range[128] = {0};
	unsigned long  rangf = 0;
	unsigned long  rangb = 0;
	if( strlen(obj->request_header.range) > 0 )
	{
		strncpy(range, obj->request_header.range, sizeof(range));
		char *p = strcasestr(range, "bytes=");
		if(NULL != p)
		{
			char temp[128] = {0};
			p += strlen("bytes=");
			char *pb = strstr(p, "-");
			if( NULL != pb)
			{
				strncpy(temp, p, pb-p);
				rangf = atol(temp);

				if(strlen(pb) > 1)
				{
					pb += 1;
					memset(temp, 0, sizeof(temp));
					strncpy(temp, pb, sizeof(temp));
					rangb = atol(temp);
				}
			}

			sw_log_debug("[%s %s %d] %d %d\n", __FUNCTION__, __FILE__, __LINE__, rangf, rangb);
		}
	}

	PLAYINFO *playinfo = httpplay_media_getfilename(file);
	HttpResponseNum response_num = HTTP_OK;
	unsigned long size = 0;
	int fd = -1;
	if(NULL != playinfo)
	{
		if(NULL != strstr(playinfo->filename, "file:") )
		{
			size = get_filesize(playinfo->filename+8);
			fd = open(playinfo->filename+8, O_RDONLY);
		}
		else
		{
			size = get_filesize(playinfo->filename);
			fd = open(playinfo->filename, O_RDONLY);
		}

		if(fd > 0 )
		{
			lseek(fd, rangf, SEEK_SET);
			if(rangb == 0 && rangf != 0)
			{
				rangb = size;
			}

			if(rangb > size )
			{
				rangb = size;
			}

			if(rangb > rangf)
			{
				size = rangb - rangf;
			}

			if(rangf >= size)
			{
				size = 0;
			}
		}

		sw_log_debug("[%s %s %d] size=%d\n", __FUNCTION__, __FILE__, __LINE__, size);
	}
	else
	{
		response_num = 	HTTP_NOT_FOUND;
		sw_log_debug("[%s %s %d] %s\n", __FUNCTION__, __FILE__, __LINE__, file);
	}

	if(sw_httpserver_send_response_header(obj, response_num, "text/image", NULL, "Keep-Alive", size, timeout) < 0)
	{
		sw_log_debug("sw_httpserver_send_response_header failed!!!\n");
	}
	else
	{
		sw_log_debug("send header ok\n");
	}

	while(size > 0)
	{
		char buf[1024] = {0};
		int len = 0;
		if(fd>0)
			len = read(fd, buf, sizeof(buf));
		sw_httpserver_send_response_content(obj, buf, len, timeout);
		size -= len;
	}

	if(fd >= 0)
		close(fd);

	sw_log_debug("[%s %s %d] GET END\n", __FUNCTION__, __FILE__, __LINE__);
	if(NULL != obj)
	{
		sw_httpserver_close_connectobj(obj);
	}

	return 0;
}


/** 
 * @brief httpplay listversion get
 * @return listversion
 */
int httpplay_listversion_get()
{
	sw_log_debug("[%s %s %d] version=%d \n", __FUNCTION__, __FILE__, __LINE__, m_STBlistVersion);
	return m_STBlistVersion;
}

/** 
 * @brief httpplay listversion set
 */
int httpplay_listversion_set(int version)
{
	m_STBlistVersion = version;

	sw_log_debug("[%s %s %d] version=%d \n", __FUNCTION__, __FILE__, __LINE__, m_STBlistVersion);
	return 0;
}


/** 
 * @brief  swmedia status 
 */
int httpplay_status_get()
{
    int httpplay_status = 0;
	printf("[%s %s %d] status = %d\n",__FUNCTION__,__FILE__,__LINE__,status);
	if(status == 3 || status == 8 || status == 7)
	{
		httpplay_status = 1;
	}
	else if(status == 4)
	{
		httpplay_status = 2;
	}

	return httpplay_status;
}

/** 
 * @brief 保存上传的图片
 * @return 成功,返回0;否则,返回-1
 */
static int httpplay_save_image(SHttpConnectObj *obj)
{

	if(NULL == obj)
	{
		return -1;
	}

	int timeout = 5000;
	if(STBBACKGROUND != httpplay_getorder(obj->request_header.request_url))
	{
		return -1;
	}

	sw_log_debug("[%s %s %d] %s\n", __FUNCTION__, __FILE__, __LINE__, obj->request_header.request_url);
	sw_log_debug("[%s %s %d] %s\n", __FUNCTION__, __FILE__, __LINE__, obj->request_header.request_url);
	sw_log_debug("[%s %s %d] %s\n", __FUNCTION__, __FILE__, __LINE__, obj->request_header.request_url);
#if 0
	if(httpplay_checkmd5(obj, obj->request_header.request_url) == -1)
	{
		//httpplay_respond(obj, HTTP_UNAUTHORIZED);
		sw_log_debug("[%s %s %d] %s\n", __FUNCTION__, __FILE__, __LINE__, obj->request_header.request_url);
		sw_log_debug("[%s %s %d] Check MD5 failed\n", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}
#endif
	HttpResponseNum response_num = HTTP_OK;
	int len = obj->request_header.content_length;
	int recv = 0;
	char *p = NULL;
	FILE *fp = NULL;
	fp = fopen("./media/tmp.image", "w");
	if(fp == NULL)
		return -1;
#if 0
	if((p = strstr(obj->request_header.request_url, "audio&")) != NULL)
	{
		p += strlen("audio&");
		memset(m_audioimage, 0, sizeof(m_audioimage));
		snprintf(m_audioimage, sizeof(m_audioimage), "/tmp/%s", p);
		sw_log_debug("[%s %s %d] %s\n", __FUNCTION__, __FILE__, __LINE__, m_audioimage);

		fp = fopen(m_audioimage, "w");
	}
	else if((p = strstr(obj->request_header.request_url, "stbimage&")) != NULL)
	{
		p += strlen("stbimage&");
		memset(m_stbimage, 0, sizeof(m_stbimage));
		snprintf(m_stbimage, sizeof(m_stbimage), "/tmp/%s", p);
		sw_log_debug("[%s %s %d] %s\n", __FUNCTION__, __FILE__, __LINE__, m_stbimage);

		fp = fopen(m_stbimage, "w");
	}
#endif

	sw_log_debug("[%s %s %d] %d\n", __FUNCTION__, __FILE__, __LINE__, len);
	while(recv < len)
	{
		char buf[1024] = {0};
		int ret = sw_httpserver_recv_request_content(obj, buf, 1024, timeout); 
		fwrite(buf, 1024, 1, fp);
		if(ret < 0)
		{
			if( NULL != obj)
			{
				sw_httpserver_close_connectobj(obj);
			}
			sw_log_debug("recv request content failed\n");
			fclose(fp);
			return -1;
		}
		recv += ret;
	}

	sw_log_debug("[%s %s %d] %d\n", __FUNCTION__, __FILE__, __LINE__, recv);
	fclose(fp);
	httpplay_respond(obj,  HTTP_OK);

	return 0;
}

/** 
 * @brief  http post 方法的回应
 * @return 成功,返回0;否则,返回-1
 */
static int httpplay_post_respond(SHttpConnectObj *obj)
{
	int timeout = 5000;
	HttpResponseNum response_num = HTTP_OK;
	int len = 0;
	char *content = (char *)malloc(1024*10*sizeof(char));
	char *buf = (char *)malloc(len*sizeof(char)+2);
	int recv = 0;
	int order = 0;
	int rt = 0;
	
	if(buf == NULL || content == NULL)
	{
		rt = -1;
		goto end;
	}
	memset(buf, 0, len+2);
	memset(content, 0, 1024*10);
	if(obj!=NULL)
		len = obj->request_header.content_length;
	while(recv < len)
	{
		int ret = sw_httpserver_recv_request_content(obj, buf+recv, len+2-recv, timeout); 
		if(ret < 0)
		{
			sw_log_debug("recv request content failed\n");
			rt = -1;
			goto end;
		}
		recv += ret;
	}
	
	order = httpplay_getorder(buf);

	sw_log_info("[%s %s %d] ORDER=%d BUF:%s\n", __FUNCTION__, __FILE__, __LINE__, order, buf);
	if(order != STBCHECK && httpplay_checkmd5(obj, buf) == -1)
	{
		sw_log_debug("[%s %s %d] Check MD5 failed\n", __FUNCTION__, __FILE__, __LINE__);
		rt = -1;
		goto end;
	}

	if(order == STBCHECK)
	{
		httpplay_checkstb_respond(obj);
		rt = 0;
		goto end;
	}

	if( order != STBMODEEXTERN && !extern_mode )
	{
		httpplay_respond(obj, HTTP_FORBIDDEN);
		rt = 0;
		goto end;
	}
	httpplay_tick = sw_thrd_get_tick();
	switch(order)
	{
		case STBMODEEXTERN:
			{
				if( !extern_mode )
				{
					httpplay_check_init();
					extern_mode = true;
				}
			}
			break;
		case STBMODESTOP:
			{
				extern_mode = false;
				sw_advertisement_start();
			}
			break;
		case STBMODERM:
			{
			}
			break;
		case STBLISTVERSION:
			break;
		case STBLIST:
			{
				char path[256] = {0};

				snprintf(path, sizeof(path), "%s/program", "./media" );
				httpplay_medialist_destroy();
				httpplay_readdir(path);
				httpplay_medialist_get(content);
				break;
			}
		case STBMEDIAPLAY:
			{
				char move[128] = {0};
				char *p = strstr(buf, "STBmedia=play:");
				p += strlen("STBmedia=play:");
				strncpy(move, p, strlen(p));
				
				PLAYINFO *playinfo = httpplay_media_getfilename(move);
				if(NULL == playinfo)
				{
					httpplay_respond(obj, HTTP_NOT_FOUND);
					rt =  -1;
					goto end;
				}

				httpplay_image_stop();
				if(playinfo->type == 3)
				{
					httpplay_audio_showimage();
				}

				if(NULL != playinfo)
				{
					char filename[128]="";
					char *p = NULL;
					if(playinfo->type == 2)
					{
						m_mediatype = 3;
					}
					else if(playinfo->type == 3)
					{
						m_mediatype = 4;
					}
					strncpy(filename,playinfo->filename,strlen(playinfo->filename));
					p = strrchr(filename,'.');
					if(p)
					{
						memset(mediatype, 0, sizeof(mediatype));
						strncpy(mediatype, p+1, strlen(p+1));
					}
					printf(">>>>>>mediatype = %s<<<<<<\n",mediatype);
					sw_media_stop();
					sw_media_play(playinfo->filename);
				}
				break;
			}
		case STBMEDIASTOP:
			httpplay_media_stop();
			break;
		case STBMEDIAPAUSE:
			sw_media_pause();
			break;
		case STBMEDIARESUME:
			sw_media_resume();
			break;
		case STBMEDIASEEK:
			{
				if(strncasecmp(mediatype,"flv",3) != 0)
				{
					int seek = 0;
					char *p = strstr(buf, "STBmedia=seek:");
					p += strlen("STBmedia=seek:");
					seek = atoi(p);
					sw_media_seek(seek*1000);
				}
				break;
			}
		case STBMEDIAINFO:
			{
				if(m_mediatype == 0)
				{
					snprintf(content, 64, "mediainfo=[0&0&0&0]");
				}
				else if(m_mediatype == 1)
				{
					snprintf(content, 64, "mediainfo=[0&0&1&1]");
				}
				else if(m_mediatype == 2)
				{
					snprintf(content, 64, "mediainfo=[0&0&1&1]");
				}
				else if(m_mediatype == 3)
				{
					int duratime = sw_media_get_duration();
					int curtime  = sw_media_get_datatime();
					int status = httpplay_status_get();
					snprintf(content, 64, "mediainfo=[%d&%d&2&%d]", curtime, duratime, status);
				}
				else if(m_mediatype == 4)
				{
					int duratime = sw_media_get_duration();
					int curtime  = sw_media_get_datatime();
					int status = httpplay_status_get();
					snprintf(content, 64, "mediainfo=[%d&%d&3&%d]", curtime, duratime, status);
				}
				else if(m_mediatype == 5)
				{
					snprintf(content, 64, "mediainfo=[0&0&0&3]");
				}
			}
			break;
		case STBVOLUMEADD:
			{
				sw_audio_inc_vol();
				sw_audiopan_draw_volume( sw_audio_get_volume(), 3000 );
			}
			break;
		case STBVOLUMEDEC:
			{
				sw_audio_dec_vol();
				sw_audiopan_draw_volume( sw_audio_get_volume(), 3000 );
			}
			break;
		case STBVOLUMEGET:
			snprintf(content, 32, "getvolume=%d", sw_audio_get_volume());
			break;
		case STBVOLUMESET:
			{
				char *p = strstr(buf, "STBmedia=setvolume");
				p += strlen("STBmedia=setvolume");
				sw_audio_set_volume(atoi(p+1));
				sw_audiopan_draw_volume(sw_audio_get_volume(), 3000);
			}
			break;
		case STBIMAGESHOWFULL:
			{
				char image[256] = {0};
				char *p = strstr(buf, "STBimageshow=showfull:");
				p += strlen("STBimageshow=showfull:");
				strncpy(image, p, strlen(p));

				PLAYINFO *playinfo = httpplay_media_getfilename(image);
				if(NULL == playinfo)
				{
					httpplay_respond(obj, HTTP_NOT_FOUND);
					rt =  -1;
					goto end;
				}

				httpplay_media_stop();
				httpplay_image_stop();
				httpplay_image_show(playinfo->filename);
			}
			break;
		case STBIMAGESHOWWINDOW:
			{
				char image[256] = {0};
				char *p = strstr(buf, "STBimageshow=showwindow:");
				p += strlen("STBimageshow=showwindow:");
				strncpy(image, p, strlen(p));


				PLAYINFO *playinfo = httpplay_media_getfilename(image);
				if(NULL == playinfo)
				{
					httpplay_respond(obj, HTTP_NOT_FOUND);
					rt = -1;
					goto end;
				}

				httpplay_media_stop();
				httpplay_image_stop();
				httpplay_image_showwindow(playinfo->filename);
			}
			break;
		case STBIMAGEEFFECT:
			{
				httpplay_media_stop();
				httpplay_image_stop();
				httpplay_imageshow_effect();
			}
			break;
		case STBIMAGESTOP:
			httpplay_image_stop();
			break;
		case STBBACKGROUND:
			httpplay_image_save(buf, obj);	
			break;
		default:
			sw_log_debug("[%s %s %d] \n", __FUNCTION__, __FILE__, __LINE__);
			break;
	}

	if(sw_httpserver_send_response_header(obj, response_num, "text/json", NULL, "Keep-Alive", strlen(content), timeout) < 0)
	{
		printf("sw_httpserver_send_response_header failed!!!\n");
	}
	else
	{
		printf("send header ok\n");
	}

	if(strlen(content) > 0)
	{
		sw_log_debug("[%s %s %d] %s\n", __FUNCTION__, __FILE__, __LINE__, content);
		sw_httpserver_send_response_content(obj, content, strlen(content), timeout);
	}

end:
	if(buf)
		free(buf);
	if(content)
		free(content);
	buf = NULL;
    content = NULL;
	sw_log_debug("[%s %s %d] POST END\n", __FUNCTION__, __FILE__, __LINE__);
	return rt;
}

/** 
 * @brief  httpserver callback
 */
static int httpplay_callback(SHttpConnectObj *obj, uint32_t lParam)
{
	int timeout = 5000;
	int ret = -1;
	HttpResponseNum response_num;

	if(NULL == obj)
		return -1;
	sw_log_debug("[%s %s %d] HTTP CALLBACK\n", __FUNCTION__, __FILE__, __LINE__);
	ret = sw_httpserver_recv_request_header(obj, timeout);
	if(ret < 0)
	{
		sw_log_fatal("recv request header failed\n");
		if(NULL !=  obj)
		{
			sw_httpserver_close_connectobj(obj);
		}

		return -1;
	}

	sw_log_debug("[%s %s %d] <method> %s\n", __FUNCTION__, __FILE__, __LINE__, obj->request_header.method);

	if(strlen(sw_advertisement_get_diskpath()) == 0)
	{
		response_num = HTTP_INTERNAL_SERVER_ERROR;
		if(sw_httpserver_send_response_header(obj, response_num, "text/image", NULL, "Keep-Alive", 0, timeout) < 0)
		{
		     sw_log_debug("sw_httpserver_send_response_header failed!!!\n");
		}

		if(NULL != obj)
		{
			sw_httpserver_close_connectobj(obj);
		}

		return -1;
	}

	if(strncasecmp(obj->request_header.method, "GET", strlen("GET")) == 0)
	{
		httpplay_get_respond(obj);
		return 0;
	}

	if(strncasecmp(obj->request_header.method, "POST", strlen("POST")) == 0)
	{

		sw_log_debug("[%s %s %d] \n", __FUNCTION__, __FILE__, __LINE__);
		if(httpplay_save_image(obj) == 0)
		{
			return 0;
		}

		httpplay_post_respond(obj);
		sw_log_debug("[%s %s %d] \n", __FUNCTION__, __FILE__, __LINE__);
		sw_log_debug("[%s %s %d] post end\n", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}
	response_num = HTTP_NOT_IMPLEMENTED;

	char buf[1024] = {0};
	ret = sw_httpserver_recv_request_content(obj, buf, sizeof(buf), timeout);

	if(sw_httpserver_send_response_header(obj, response_num, "text/image", NULL, "Keep-Alive", 0, timeout) < 0)
	{
		printf("sw_httpserver_send_response_header failed!!!\n");
	}
	else
	{
		printf("send header ok\n");
	}

	if(NULL != obj)
	{
		sw_httpserver_close_connectobj(obj);
	}

	return -1;
}

/** 
 * @brief from playname get filename
 * @param playname PLAYINFO.playname
 * @return 成功,返回 PLAYINFO;否则,返回 NULL
 */
static PLAYINFO *httpplay_media_getfilename(char *playname)
{
	if(NULL == playname || NULL == playinfo_head)
	{
		sw_log_debug("[%s %s %d] Filename or Head is NULL\n", __FUNCTION__, __FILE__, __LINE__);
		return NULL;
	}

	PLAYINFO *playinfo = playinfo_head;
	while(NULL != playinfo)
	{
		if(strncmp(playname, playinfo->playname, strlen(playname) ) == 0)
		{
			sw_log_debug("[%s %s %d] %s %s\n", __FUNCTION__, __FILE__, __LINE__, playinfo->playname, playinfo->filename);
			return playinfo;
		}
		playinfo = playinfo->next;
	}

	return NULL;
}

/** 
 * @brief show all picture by skia effect
 * @return 成功,返回0;否则,返回-1
 */
int httpplay_imageshow_effect()
{
	return 0;
}


/** 
 * @brief list add node
 * @return 成功,返回0;否则,返回-1
 */
int httpplay_media_add(PLAYINFO *playinfo)
{
	if(NULL == playinfo)
	{
		sw_log_debug("[%s %s %d] Playinfo is NULL!\n", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}

	sw_log_debug("[%s %s %d] %d %s %s\n", __FUNCTION__, __FILE__, __LINE__, playinfo->type, playinfo->playname, playinfo->filename);
	if(NULL == playinfo_head )
	{
		playinfo_head = playinfo;
		return 0;
	}

	PLAYINFO *p = playinfo_head;
	while( p->next != NULL)
	{
		if(strncmp(p->filename, playinfo->filename, strlen(p->filename)) == 0)
		{
			sw_log_debug("[%s %s %d] %s is already exist\n", __FUNCTION__, __FILE__, __LINE__, p->playname);
			return 1;
		}
		p = p->next;
	}

	p->next = playinfo;
	return 0;
}

/** 
 * @brief  读取文件到内存
 * @param  filepath  文件名
 * @return 返回 内存地址
 */
char *httpplay_parser_read_file(char *filepath)
{
	FILE *fp = NULL;
	char *filebuf = NULL;
	int filesize = 0;

	if(NULL == filepath)
	{
		sw_log_debug("[%s %s %d] filepath is NULL  \n", __FUNCTION__, __FILE__, __LINE__);
		return NULL;
	}

	sw_log_debug("[%s %s %d] filepath is %s  \n", __FUNCTION__, __FILE__, __LINE__, filepath);
	if( (fp = fopen(filepath,"r")) != NULL )
	{
		fseek(fp, 0, SEEK_END);
		filesize = ftell(fp);
		if(filesize < 0)
		{
			fclose(fp);
			return NULL;
		}
		fseek(fp, 0, SEEK_SET);
		filebuf = (char *)malloc(filesize+1);

		if(NULL != filebuf)
		{	
			memset(filebuf, 0, filesize+1);
			fread(filebuf, filesize, 1, fp);
		}

		fclose(fp); 
	}
	return filebuf;
}

/** 
 * @brief 解析 page.xml 文件
 * @return 成功,返回0;否则,返回-1
 */
static int  httpplay_parse_page(char *path)
{
	return 0;
}

/** 
 * @brief 遍历目录，寻找page.xml
 * @param path ,目录
 * @return 成功,返回0;否则,返回-1
 */
int httpplay_readdir(char *path)
{
	DIR *dir;
	struct dirent *ptr;
	if(NULL == path)
	{
		sw_log_debug("[%s %s %d] path is NULL !\n", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}

	dir = opendir(path);
	if(dir == NULL)
		return -1;
	while((ptr = readdir(dir)) != NULL)
	{
		if(ptr->d_type == 4 && strncmp(ptr->d_name, "." ,1) != 0 && strncmp(ptr->d_name, "..", 2) != 0)
		{
			char temp[256] = {0};
			sprintf(temp, "%s/%s", path, ptr->d_name);
			sw_log_debug("[%s %s %d] d_name:%s\n", __FUNCTION__, __FILE__, __LINE__, temp);
			httpplay_readdir(temp);
		}
		else if(ptr->d_type == 8)
		{
			if(strncmp("page.xml", ptr->d_name, strlen(ptr->d_name)) == 0)
			{
				char temp[256] = {0};
				sprintf(temp, "%s", path);
				sw_log_debug("[%s %s %d] page.xml:%s/page.xml\n", __FUNCTION__, __FILE__, __LINE__, path);
				httpplay_parse_page( temp);
			}
		}
	}

	closedir(dir);
	return 0;
}

/** 
 * @brief 打印所有的media信息
 * @return 成功,返回0;否则,返回-1
 */
int httpplay_media_printinfo()
{
	if(NULL == playinfo_head)
	{
		sw_log_debug("[%s %s %d] no media info!\n", __FUNCTION__, __FILE__, __LINE__);
	}

	PLAYINFO *playinfo = playinfo_head;
	while(NULL != playinfo)
	{
		sw_log_debug("<playinfo> type:%d playname:%s filename:%s\n", playinfo->type, playinfo->playname, playinfo->filename);
		playinfo = playinfo->next;
	}

	return 0;
}
