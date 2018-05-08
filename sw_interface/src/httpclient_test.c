#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "swapi_linux.h"
#include "swtype.h"
#include "swhttpfile.h"


int main(int argc, char *argv[])
{
	int timeout = 5000;
	char *url = "http://192.168.1.108:80/media.json";
	//char *url = "http://192.168.1.108/program/5C48F9EDE83715655508341B5AA04D65/Page_0/html/index.html";
	//char *url = "http://apis.map.qq.com/ws/geocoder/v1/?address=北京市海淀区彩和坊路海淀西大街74号&key=OB4BZ-D4W3U-B7VVO-4PJWW-6TKDJ-WPB77";
//	char *url = "http://apis.map.qq.com/uri/v1/search?keyword=酒店&region=北京&referer=myapp";
	char *filebuf;
	HANDLE httpfile = sw_httpfile_init(url, timeout);
	if(httpfile == NULL)
	{
		printf("open http file error\n");
	}	
	int filesize = sw_httpfile_get_size(httpfile);
	if(filesize <= 0)
	{
		printf("filesize <= 0\n");
	}

	filebuf = (char*)malloc(filesize);
	if(filebuf == NULL)
	{
		printf("filebuf == NULL\n");
	}
	filesize = sw_httpfile_get_file(httpfile,filebuf,filesize,timeout);
	printf("swhttpfile_get_file:%s\n", filebuf);
	if(NULL == filebuf)
	{
		free(filebuf);
	}
	if(NULL != httpfile)
	{
		sw_httpclient_disconnect(httpfile);
	}
	return 0;
}
