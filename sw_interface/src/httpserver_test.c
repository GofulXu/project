#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "swapi_linux.h"
#include "swtype.h"
#include "swhttpserver.h"
#include "md5.h"

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
	int type;
	char filename[128];
	char playname[128];
	char country[32];
	struct playinfo_t *next;
}PLAYINFO;

static char m_mac[32];
static char mediatype[128] = "";
static PLAYINFO *playinfo_head = NULL;

static PLAYINFO *httpplay_media_getfilename(char *playname)
{
	if(NULL == playname || NULL == playinfo_head)
	{
		printf("[%s %s %d] filename or head is NULL\n", __FUNCTION__, __FILE__, __LINE__);
		return NULL;
	}

	PLAYINFO *playinfo = playinfo_head;
	while(NULL != playinfo)
	{
		if(strncmp(playname, playinfo->playname, strlen(playname)) == 0)
		{
			printf("[%s %s %d] %s %s\n", __FUNCTION__, __FILE__, __LINE__, playinfo->playname, playinfo->filename);
			return playinfo;
		}
		playinfo = playinfo->next;
	}
	return NULL;
}

static void check_md5(char *dkey, char *filebuf, int size)
{
	struct SWMD5Context md5ctx;
	char buf[128];
	memset(buf, 0, sizeof(buf));
	unsigned char key[16];

	memset(key, 0, sizeof(key));
	SWMD5Init(&md5ctx);
	SWMD5Update(&md5ctx, (unsigned char const*)filebuf, size);
	SWMD5Final(key, &md5ctx);

	int i = 0;
	for(i = 0; i < 16; i++)
	{
		sprintf(dkey+i*2, "%02x", key[i]);
	}
	printf("[%s %s %d buf:%s md5:%s\n]", __FUNCTION__, __FILE__, __LINE__, filebuf, dkey);
}

static int httpplay_checkmd5(SHttpConnectObj *obj, char *buf)
{
	struct sockaddr_in from;
	char client_ip[128] = {0};
	char md5_ip[128] = {0};
	char buffer[256] = {0};
	char session_key[256] = {0};
	bool bMd5 = false;

	if(NULL == buf || NULL == obj)
	{
		printf("[%s %s %d]error\n", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}

	from.sin_addr.s_addr = obj->from_ip;
	strcpy(client_ip, inet_ntoa(from.sin_addr));

	printf("{%s %s %d} client ip: %s\n", __FUNCTION__, __FILE__, __LINE__, client_ip);
	check_md5(md5_ip, client_ip, strlen(client_ip));

	sprintf(buffer, "%s%s", m_mac, md5_ip);
	printf("{%s %s %d} buffer: %s\n", __FUNCTION__, __FILE__, __LINE__, buffer);
	check_md5(session_key, buffer, strlen(buffer));
	printf("HTTPPLAY MD5:%s\n", session_key);
	char *p = strstr(buf, "KEY=");
	if(p != NULL)
	{
		if(strncmp(p+4, session_key, strlen(session_key)) == 0)
		{
			bMd5 = true;
			printf("{%s %s %d} check md5 ok %s\n", __FUNCTION__, __FILE__, __LINE__);
			return 0;
		}
	}
	
	char content[256] = {0};
	int timeout = 5000;
	HttpResponseNum response_num = HTTP_UNAUTHORIZED;
	printf("{%s %s %d} check md5 failed\n", __FUNCTION__, __FILE__, __LINE__);
	sprintf(content, "the correct md5 is %s\r\nthe mac is %s\r\nthe md5 ip is %s\r\n", session_key, m_mac, md5_ip);
	int len = strlen(content);
	if(sw_httpserver_send_response_header(obj, response_num, "doc/html;charset=utf-8", NULL, "Keep-Alive", len, timeout) <= 0)
	{
		printf("{%s %s %d}\n", __FUNCTION__, __FILE__, __LINE__);
		
	}

	if(sw_httpserver_send_response_content(obj, content, len, timeout) < 0)	
	{
		printf("{%s %s %d}\n", __FUNCTION__, __FILE__, __LINE__);
	}

	if(obj != NULL)
	{
		sw_httpserver_close_connectobj(obj);
	}
	return -1;
}

unsigned long get_filesize(const char *filename)
{
	if(NULL == filename)return -1;

	struct stat buf;
	if(stat(filename, &buf) < 0)
	{
		perror("stat");
		return -1;
	}
	printf("%s:%ld bytes\n", filename, buf.st_size);
	return buf.st_size;
}

static int httpplay_get_repond(SHttpConnectObj *obj)
{
	printf("hello get\n");
	if(NULL == obj)return -1;

	char file[128] = {0};
	int timeout = 5000;
	char *p = strcasestr(obj->request_header.request_url, "?KEY");
	if(NULL != p)
	{
		strncpy(file,obj->request_header.request_url+1, p - obj->request_header.request_url - 1);
	}

	if(httpplay_checkmd5(obj, obj->request_header.request_url) == -1)
	{
		printf("check md5 error \n");
		return 0;
	}

	char range[128] = {0};
	unsigned long rangf = 0;
	unsigned long rangb = 0;
#if 0
	if(strlen(obj->request_header.range) > 0)
	{
		strncpy(range, obj->request_header.range, sizeof(range));
		char *p = strcasestr(range,"bytes=");
		if(NULL != p)
		{
			char temp[128] = {0};
			p += strlen("bytes=");
			char *pb = strstr (p, "-");
			if(NULL !=pb)
			{
				strncpy(temp, p, pb - p);
				rangf = atol(temp);

				if(strlen(pb) > 1)
				{
					pb += 1;
					memset(temp, 0, sizeof(temp));
					strncpy(temp, pb, sizeof(temp));
					rangb = atol(temp);
				}
			}
			printf("[%s %s %d] %d %d\n", __FUNCTION__, __FILE__, __LINE__, rangf, rangb);
		}
	}
#endif
	PLAYINFO *playinfo = httpplay_media_getfilename(file);
	HttpResponseNum response_num = HTTP_OK;
	unsigned long size = 0;
	int fd = -1;
	if(NULL != playinfo)
	{
		if(NULL != strstr(playinfo->filename, "file:"))
		{
			size = get_filesize(playinfo->filename+8);
			fd = open(playinfo->filename+8, O_RDONLY);
		}else
		{
			size = get_filesize(playinfo->filename);
			fd = open(playinfo->filename, O_RDONLY);
		}

		if(fd > 0)
		{
			lseek(fd, rangf, SEEK_SET);
			if(rangb == 0 && rangf != 0)
			{
				rangb = size;
			}

			if(rangb > size)
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
			printf("[%s %s %d] size = %d\n", __FUNCTION__, __FILE__, __LINE__, size);

	}else
	{
		response_num = HTTP_NOT_FOUND;
		printf("[%s %s %d] size = %s\n", __FUNCTION__, __FILE__, __LINE__, file);

		
	}
	
	if(sw_httpserver_send_response_header(obj, response_num, "doc/image", NULL, "Keep-Alive", size, timeout) < 0)
	{
		printf("sw_httpserver_send_response_header failed!!!\n");
	}else
	{
		printf("send header ok\n");
	}

	while(size > 0)
	{
		char buf[1024] = {0};
		int len = 0;
		if(fd > 0 )
			len = read(fd, buf, sizeof(buf));
		sw_httpserver_send_response_content(obj, buf, len, timeout);
		size -= len;
	}

	if(fd > 0)
	{
		close(fd);
	}

	printf("[%s %s %d] GET END\n", __FUNCTION__, __FILE__, __LINE__);
	if(NULL != obj)
	{
		sw_httpserver_close_connectobj(obj);
	}


	return 0;
}
static int httpplay_post_respond(SHttpConnectObj *m_obj)
{
	printf("hello post \n");
	return 0;
}
static int httpplay_save_image(SHttpConnectObj *m_obj)
{
	printf("hello saveimage\n");
	return 0;
}
int m_httpserver_callback(SHttpConnectObj* m_obj, uint32_t m_mwparam)
{
	printf("goeful = %d\n", 111);
	char m_buf[1024] = {0};
	int ret = -1;
	ret = sw_httpserver_recv_request_header(m_obj, 5000);
	if(ret < 0){
		printf("http_server_request_header error s_recv = %d\n", ret);
		if(NULL != m_obj)sw_httpserver_close_connectobj(m_obj);
		return -1;
	}

	printf("\tmethod:%s\n", m_obj->request_header.method);
	printf("\trequest url:%s\n", m_obj->request_header.request_url);
	printf("\thost:%s\n", m_obj->request_header.host);
	printf("\taccept type:%s\n", m_obj->request_header.accept_type);
	printf("\taccept encoding:%s\n", m_obj->request_header.accept_encoding);
	printf("\tconnection:%s\n", m_obj->request_header.connection);
	printf("\tauthorization:%s\n", m_obj->request_header.authorization);
	printf("\tcontent type:%s\n", m_obj->request_header.content_type);
	printf("\tcontent length:%d\n", m_obj->request_header.content_length);
	printf("\theader length:%d\n", m_obj->request_header.header_length);

	if(strncasecmp(m_obj->request_header.method, "GET", strlen("GET")) == 0)	
	{
		httpplay_get_repond(m_obj);
		return 0;
	}

	if(strncasecmp(m_obj->request_header.method, "POST", strlen("POST")) == 0)
	{
		if(httpplay_save_image(m_obj) == 0)
		{
			return 0;
		}
		httpplay_post_respond(m_obj);
		return 0;
	}

	ret = sw_httpserver_recv_request_content(m_obj,m_buf, sizeof(m_buf), 5000);
	if(sw_httpserver_send_response_header(m_obj, 501, "doc/image", NULL, "Keep-Alive", 0, 5000) < 0)
	{
		printf("sw_httpserver_send_response_header failed!!!\n");
	}else
	{
		printf("send header ok\n");

	}
	if(NULL != m_obj)
	{
		sw_httpserver_close_connectobj(m_obj);
	}
	return -1;
}

int main(int argc, char *argv[])
{
	HANDLE http_sfd = sw_httpserver_open(htons((unsigned short)(9999)), m_httpserver_callback, 0);
	
	while(1);
	sw_httpserver_close(http_sfd);
	return 0;
}
