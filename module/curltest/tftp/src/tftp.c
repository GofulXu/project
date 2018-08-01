#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "curl.h"

int gf_tftp_downloadfile(char *ip, char *savepath, char *filename, int timeout)
{
	FILE *fd;
	char url[256] = {0};
	CURL *curl;
	if(0 >= timeout)
		timeout = 60*60*1000;

	if(savepath)
	{
		if(access(savepath, F_OK) == 0)
			remove(savepath);
		fd = fopen(savepath, "w+");
	}else
	{
		if(access(filename, F_OK) == 0)
			remove(filename);
		fd = fopen(filename, "w+");
	}
	sprintf(url, "tftp://%s/%s", ip, filename);
	curl = curl_easy_init();
	if(curl)
	{
		printf("url:%s\n", url);
		curl_easy_setopt(curl, CURLOPT_URL, url);
#ifdef DEBUG
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif
		curl_easy_setopt(curl, CURLOPT_TFTP_BLKSIZE, 2048L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);
		if(curl_easy_perform(curl) == CURLE_OK )
		{
			fclose(fd);
			curl_easy_cleanup(curl);
			return 0;
		}
		curl_easy_cleanup(curl);
	}
	fclose(fd);
	if(savepath)
		remove(savepath);
	else
		remove(filename);
	return -1;
}


int gf_tftp_uploadfile(char *ip, char *filepath, int timeout)
{

	FILE *fd;
	CURL *curl;
	char url[256] = {0};
	if(access(filepath, F_OK) != 0)
		return -1;
	int len = strlen(filepath);
	if(filepath[len] == '/')
		return -1;
	while(filepath[len-1] != '/')
	{
		if(len == 0)
			break;
		len--;
	}
	if(0 >= timeout)
		timeout = 60*60*1000;
	sprintf(url, "tftp://%s/%s", ip, filepath + (len));
	fd = fopen(filepath, "r");
	curl = curl_easy_init();
	if(curl)
	{
		printf("url:%s\n", url);
		curl_easy_setopt(curl, CURLOPT_URL, url);
#ifdef DEBUG
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif
		curl_easy_setopt(curl, CURLOPT_TFTP_BLKSIZE, 2048L);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curl, CURLOPT_READDATA, fd);
		if(CURLE_OK == curl_easy_perform(curl))
		{
			curl_easy_cleanup(curl);
			fclose(fd);
			return 0;
		}
		curl_easy_cleanup(curl);
	}
	fclose(fd);
	return -1;
}

int main(int argc, char *argv[])
{
	int success = -1;
	if(argc < 2)
	{
		printf("pasge error\n");
		printf("please application -g + filename to download\t -p + filename to upload\n");
		return 0;
	}
	else if(0 == strcmp(argv[1],"-h") || NULL != strstr(argv[1], "--h"))
	{
		printf("please application -g + filename to download\t -p + filename to upload\n");
		return 0;
	}
	if(argc != 4)
	{
		printf("pasge error");
		printf("please application -g + filename to download\t -p + filename to upload\n");
		return 0;
	}
	if(strcmp(argv[1], "-g") == 0)
		success = gf_tftp_downloadfile(argv[3], NULL, argv[2], 0);
	if(strcmp(argv[1], "-p") == 0)
		success = gf_tftp_uploadfile(argv[3], argv[2], 0);
	if(success == 0)
	{
		printf("upload %s success\n", argv[2]);
		return 0;
	}
	printf("upload or download error ip:%s file:%s\n", argv[3], argv[2]);
	return 0;
}
