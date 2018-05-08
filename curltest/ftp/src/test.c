#include <stdlib.h>  
#include <string.h>
#include <stdio.h>  
#include <curl/curl.h>  
#include <sys/stat.h>  

static long mem_filesize = 0;  
static long m_memsize = 0;  
enum printfmodule{
	MVERBOSE = 0X01,
	MHEADER = 0X02,
	MNOPROGRESS = 0X04
};

/* parse headers for Content-Length */  
size_t getuploadlengthfunc(void *ptr, size_t size, size_t nmemb, void *stream)   
{  
    int r;  
    long len = 0;  
    r = sscanf((const char*)ptr, "Content-Length: %ld\n", &len);  
    if (r) /* Microsoft: we don't read the specs */  
        *((long *) stream) = len;  
    return size * nmemb;  
}  
size_t getdownloadlengthfunc(void *ptr, size_t size, size_t nmemb, void *stream)   
{  
    int r;  
    long len = 0;  
//	printf("head:%s", (char*)ptr);
	r = sscanf((const char*)ptr, "213 %ld\n", &len);  
    if (r) /* Microsoft: we don't read the specs */  
        *((long *) stream) = len;  
    return size * nmemb;  
}  
/* discard downloaded data */  
size_t discardfunc(void *ptr, size_t size, size_t nmemb, void *stream)   
{  
    return size * nmemb;  
}  
//write data to upload  
size_t writefunc(void *ptr, size_t size, size_t nmemb, void *stream)  
{  
    return fwrite(ptr, size, nmemb, (FILE *)stream);  
}  
//write data to mem
static char *m_downmem = NULL;
static unsigned long m_downsize = 0;
size_t writetomemfunc(void *ptr, size_t size, size_t nmemb, void *stream)  
{  
	if(m_downmem == NULL)
	{
		m_downmem = (char*)malloc( mem_filesize);
		memset(m_downmem, 0, mem_filesize);
	}
	m_downsize += size*nmemb;
	if(m_downsize <= mem_filesize)
	{
		memcpy(m_downmem, ptr, size*nmemb);
		printf("size:%ld-%ld\t%s:%p:%s\n", size*nmemb, mem_filesize, __FUNCTION__, m_downmem, (char *)m_downmem);
		m_downmem += size*nmemb;
	}
	return size * nmemb;
}  
size_t readmemfunc(void *ptr, size_t size, size_t nmemb, void *stream)  
{

	printf("test:%s:%s\n", __FUNCTION__, (char *)stream);
	ptr = (char *)stream;
	if(m_memsize > size*nmemb)
	{
		m_memsize -= size*nmemb;
		return size*nmemb;	
	}
	else
	{
		int size = m_memsize;
		m_memsize = 0;
		return size;//size*nmemb;
	}
}
/* read data to upload */  
size_t readfunc(void *ptr, size_t size, size_t nmemb, void *stream)  
{  
    FILE *f = (FILE*)stream;  
    size_t n;  
    if (ferror(f))  
        return CURL_READFUNC_ABORT;  
    n = fread(ptr, size, nmemb, f) * size;  
    return n;  
}  
int ftp_get_remotesize(CURL *curlhandle)
{
	int r;
    long filesize =0 ;  
    //设置头处理函数  
    curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION, getdownloadlengthfunc);  
    curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA, &filesize);  
    r = curl_easy_perform(curlhandle);  

    if (r == CURLE_OK)  
        return filesize;  
    else {  
        fprintf(stderr, "%s\n", curl_easy_strerror(r));  
        return 0;  
	}
}

int ftp_set_headopt(CURL *curlhandle, const char * remotepath, const char * username, const char * password, long timeout)  
{
    curl_easy_setopt(curlhandle, CURLOPT_URL, remotepath);  
    curl_easy_setopt(curlhandle, CURLOPT_USERNAME, username);     
    curl_easy_setopt(curlhandle, CURLOPT_PASSWORD, password);
    //连接超时设置  
    curl_easy_setopt(curlhandle, CURLOPT_CONNECTTIMEOUT, timeout);  
	return 0;
}

int ftp_upload_from_file(CURL *curlhandle, const char * localpath, long tries)  
{  
    FILE *f;  
    long uploaded_len = 0;  
    CURLcode r = CURLE_GOT_NOTHING;  
    int c;  
    f = fopen(localpath, "rb");  
    if (f == NULL) {  
        perror(NULL);  
        return -1;  
    }  
    curl_easy_setopt(curlhandle, CURLOPT_UPLOAD, 1L);  
    curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION, getuploadlengthfunc);  
    curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA, &uploaded_len);  
    curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, discardfunc);  
    curl_easy_setopt(curlhandle, CURLOPT_READFUNCTION, readfunc);  
    curl_easy_setopt(curlhandle, CURLOPT_READDATA, f);  
    curl_easy_setopt(curlhandle, CURLOPT_FTPPORT, "-"); /* disable passive mode */  
    curl_easy_setopt(curlhandle, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);  
    for (c = 0; (r != CURLE_OK) && (c < tries); c++) {  
        /* are we resuming? */  
        if (c) { /* yes */  
            /* determine the length of the file already written */  
            /* 
            * With NOBODY and NOHEADER, libcurl will issue a SIZE 
            * command, but the only way to retrieve the result is 
            * to parse the returned Content-Length header. Thus, 
            * getcontentlengthfunc(). We need discardfunc() above 
            * because HEADER will dump the headers to stdout 
            * without it. 
            */  
//            curl_easy_setopt(curlhandle, CURLOPT_NOBODY, 1L);  
//            curl_easy_setopt(curlhandle, CURLOPT_HEADER, 1L);  
            r = curl_easy_perform(curlhandle);  
            if (r != CURLE_OK)  
                continue;  
//            curl_easy_setopt(curlhandle, CURLOPT_NOBODY, 0L);  
//            curl_easy_setopt(curlhandle, CURLOPT_HEADER, 0L);  
            fseek(f, uploaded_len, SEEK_SET);  
            curl_easy_setopt(curlhandle, CURLOPT_APPEND, 1L);  
        }  
        else { /* no */  
            curl_easy_setopt(curlhandle, CURLOPT_APPEND, 0L);  
        }  
        r = curl_easy_perform(curlhandle);  
    }  
    fclose(f);  
    if (r == CURLE_OK)  
        return 0;  
    else {  
        fprintf(stderr, "%s\n", curl_easy_strerror(r));  
        return -1;  
    }  
}  

int ftp_upload_from_mem(CURL *curlhandle, const char *membuf, unsigned long size, long tries)  
{  
    long uploaded_len = 0;  
    CURLcode r = CURLE_GOT_NOTHING;  
    int c;  
	m_memsize = size;
    curl_easy_setopt(curlhandle, CURLOPT_UPLOAD, 1L);  
    curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION, getuploadlengthfunc);  
    curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA, &uploaded_len);  
    curl_easy_setopt(curlhandle, CURLOPT_FTPPORT, "-"); /* disable passive mode */  
    curl_easy_setopt(curlhandle, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);  
    curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, discardfunc);  
    curl_easy_setopt(curlhandle, CURLOPT_READFUNCTION, readmemfunc);  
    curl_easy_setopt(curlhandle, CURLOPT_READDATA, membuf);  
	printf("%s:%s\n", __FUNCTION__, membuf);
#if 1
    for (c = 0; (r != CURLE_OK) && (c < tries); c++) {  
        /* are we resuming? */  
        if (c) { /* yes */  
            /* determine the length of the file already written */  
            /* 
            * With NOBODY and NOHEADER, libcurl will issue a SIZE 
            * command, but the only way to retrieve the result is 
            * to parse the returned Content-Length header. Thus, 
            * getcontentlengthfunc(). We need discardfunc() above 
            * because HEADER will dump the headers to stdout 
            * without it. 
            */  
//            curl_easy_setopt(curlhandle, CURLOPT_NOBODY, 1L);  
//            curl_easy_setopt(curlhandle, CURLOPT_HEADER, 1L);  
            r = curl_easy_perform(curlhandle);  
            if (r != CURLE_OK)  
                continue;  
//            curl_easy_setopt(curlhandle, CURLOPT_NOBODY, 0L);  
//            curl_easy_setopt(curlhandle, CURLOPT_HEADER, 0L);  
			if(uploaded_len < size)
			{
				membuf += uploaded_len;
				size -= uploaded_len;
			}
            curl_easy_setopt(curlhandle, CURLOPT_APPEND, 1L);  
        }  
        else { /* no */  
            curl_easy_setopt(curlhandle, CURLOPT_APPEND, 0L);  
        }  
        r = curl_easy_perform(curlhandle);  
    }  
#endif
    if (r == CURLE_OK)  
        return 0;  
    else {  
        fprintf(stderr, "%s\n", curl_easy_strerror(r));  
        return -1;  
    }  
}  

// 下载  
char *ftp_download_to_mem(CURL *curlhandle, long *download_size, long tries)  
{  
	mem_filesize = 0;
    curl_off_t local_file_len = -1 ;  
    CURLcode r = CURLE_GOT_NOTHING;  
    int use_resume = 0;  
	m_downmem = NULL;
TAIL:
    curl_easy_setopt(curlhandle, CURLOPT_UPLOAD, 0L);  
    //设置头处理函数  
    curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION, getdownloadlengthfunc);  
    curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA, &mem_filesize);  
    // 设置断点续传  
    curl_easy_setopt(curlhandle, CURLOPT_RESUME_FROM_LARGE, use_resume?local_file_len:0);  
    curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, writetomemfunc);  
//    curl_easy_setopt(curlhandle, CURLOPT_WRITEDATA, m_downloadmem);  

    r = curl_easy_perform(curlhandle);  
    if (r == CURLE_OK)  
	{
		*download_size = mem_filesize;
		m_downmem -= m_downsize;
	}
    else {  
        fprintf(stderr, "%s\n", curl_easy_strerror(r));  
        use_resume = 1;  
		if(tries > 0)
		{
			tries--;
			goto TAIL;
		}
    }  
	return m_downmem;  
}  

// 下载  
int ftp_download_to_file(CURL *curlhandle, const char * localpath, long tries)  
{  
    FILE *f;  
    curl_off_t local_file_len = -1 ;  
    CURLcode r = CURLE_GOT_NOTHING;  
    struct stat file_info;  
    int use_resume = 0;  
TAIL:
    //获取本地文件大小信息  
    if(stat(localpath, &file_info) == 0 && use_resume)  
    {  
        local_file_len = file_info.st_size;   
		//追加方式打开文件，实现断点续传  
		f = fopen(localpath, "ab+");  
		if (f == NULL) {  
			perror(NULL);  
			return 0;  
		}  
    }else
	{
		f = fopen(localpath, "wb+");  
		if (f == NULL) {  
			perror(NULL);  
			return 0;  
		}  
	}	
    curl_easy_setopt(curlhandle, CURLOPT_UPLOAD, 0L);  
    // 设置断点续传  
    curl_easy_setopt(curlhandle, CURLOPT_RESUME_FROM_LARGE, use_resume?local_file_len:0);  
    curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, writefunc);  
    curl_easy_setopt(curlhandle, CURLOPT_WRITEDATA, f);  
    long filesize =0 ;  
    //设置头处理函数  
    curl_easy_setopt(curlhandle, CURLOPT_HEADERFUNCTION, getdownloadlengthfunc);  
    curl_easy_setopt(curlhandle, CURLOPT_HEADERDATA, &filesize);  

    r = curl_easy_perform(curlhandle);  
    fclose(f);  
    if (r == CURLE_OK)  
        return filesize;  
    else {  
        fprintf(stderr, "%s\n", curl_easy_strerror(r));  
        use_resume = 1;  
		if(tries > 0)
		{
			tries--;
			goto TAIL;
		}
        return -1;  
    }  
}  


int ftp_set_printfmodule(CURL *curlhandle, int out)
{
	//关闭显示详细信息
	curl_easy_setopt(curlhandle, CURLOPT_VERBOSE, out&MVERBOSE?1:0);
	//关闭在正文输出中包含标题
	curl_easy_setopt(curlhandle, CURLOPT_HEADER, out&MHEADER?1:0);
	//关闭进度计
	curl_easy_setopt(curlhandle, CURLOPT_NOPROGRESS, out&MNOPROGRESS?1:0);

	return 0;
}

#define REMOTE_FTP_URL "ftp://192.168.0.108/"

char *gf_ftp_download_to_mem(const char *ftpurl, long *msize, const char *username, const char *password, const int timeout)
{
    CURL *curlhandle = NULL;  
	char ftp_url[512];
	char *mem  = NULL;
	char *m_downloadbuf = NULL;
	memset(ftp_url, 0, sizeof(ftp_url));
	strcpy(ftp_url, ftpurl);

    curl_global_init(CURL_GLOBAL_ALL);  
    curlhandle = curl_easy_init();  

	ftp_set_printfmodule(curlhandle, MNOPROGRESS);
	ftp_set_headopt(curlhandle, ftp_url, username, password, timeout);
	mem = ftp_download_to_mem(curlhandle, msize, 3);
	if(msize > 0 && mem != NULL)
	{
		printf("download suc\turl[%s]\tfilesize:%ld\nmem:%s\n", ftp_url, *msize, mem);
		m_downloadbuf = (char *)malloc(*msize);
		memset(m_downloadbuf, 0, *msize);
		memcpy(m_downloadbuf, mem, *msize);
	}
	else
		printf("download err\turl[%s]\tfilesize:%ld\n", ftp_url, *msize);

    curl_easy_cleanup(curlhandle);  
    curl_global_cleanup();  

	if(m_downmem != NULL)
	{
		free(m_downmem);
		m_downmem = NULL;
	}

	return m_downloadbuf;
}


int gf_ftp_download_to_file(const char *ftpurl, const char *local_name, const char *username, const char *password, const int timeout)
{
    CURL *curlhandle = NULL;  
	char ftp_url[512], localname[128];
	memset(ftp_url, 0, sizeof(ftp_url));
	strcpy(ftp_url, ftpurl);
	memset(localname, 0, sizeof(localname));
	strcpy(localname, local_name);

    curl_global_init(CURL_GLOBAL_ALL);  
    curlhandle = curl_easy_init();  

	ftp_set_printfmodule(curlhandle, MNOPROGRESS);
	ftp_set_headopt(curlhandle, ftp_url, username, password, timeout);
	int size = ftp_download_to_file(curlhandle, localname, 3);
	if(size > 0)
		printf("download suc\turl[%s]\tfilesize:%d\n", ftp_url, size);
	else
		printf("download err\turl[%s]\tfilesize:%d\n", ftp_url, size);

    curl_easy_cleanup(curlhandle);  
    curl_global_cleanup();  

	return size;
}

int gf_ftp_upload_from_file(const char *ftpurl, const char *local_name, const char *username, const char *password, const int timeout)
{
    CURL *curlhandle = NULL;  
	char ftp_url[512], localname[128];
	memset(ftp_url, 0, sizeof(ftp_url));
	strcpy(ftp_url, ftpurl);
	memset(localname, 0, sizeof(localname));
	strcpy(localname, local_name);

    curl_global_init(CURL_GLOBAL_ALL);  
    curlhandle = curl_easy_init();  

	ftp_set_printfmodule(curlhandle, MNOPROGRESS);
	ftp_set_headopt(curlhandle, ftp_url, username, password, timeout);
	int ret = ftp_upload_from_file(curlhandle, localname, 3); 
	if(ret == 0)
		printf("upload suc\turl[%s]\tlocalfile:%s\n", ftp_url, localname);
	else
		printf("upload err\turl[%s]\tlocalfile:%s\n", ftp_url, localname);

    curl_easy_cleanup(curlhandle);  
    curl_global_cleanup();  

	return ret;
}

int gf_ftp_upload_from_mem(const char *ftpurl, const char *membuf, unsigned long size, const char *username, const char *password, const int timeout)
{
    CURL *curlhandle = NULL;  
	char ftp_url[512];
	memset(ftp_url, 0, sizeof(ftp_url));
	strcpy(ftp_url, ftpurl);

    curl_global_init(CURL_GLOBAL_ALL);  
    curlhandle = curl_easy_init();  

	ftp_set_printfmodule(curlhandle, MNOPROGRESS);
	ftp_set_headopt(curlhandle, ftp_url, username, password, timeout);
	int ret = ftp_upload_from_mem(curlhandle, membuf, size, 3); 
	if(ret == 0)
		printf("upload suc\turl[%s]\n", ftp_url);
	else
		printf("upload err\turl[%s]\n", ftp_url);

    curl_easy_cleanup(curlhandle);  
    curl_global_cleanup();  

	return ret;
}


int main(int argc, char *argv[])   
{  
	char ftp_url[1024];
	char localname[128];
	if(argc < 2)
	{
		printf("input error\n");
		return 0;
	}
	memset(localname, 0, sizeof(localname));
	if(argc == 2)
		strncpy(localname, argv[1], sizeof(localname));
	else
		strncpy(localname, argv[2], sizeof(localname));

	printf("localname:%s\n", localname);

	memset(ftp_url, 0, sizeof(ftp_url));
	snprintf(ftp_url, sizeof(ftp_url), "%s%s", REMOTE_FTP_URL, argv[1]);

	gf_ftp_download_to_file(ftp_url, localname, "goeful", "goeful", 1000);
#if 1
	long size = 0;
	char *mem = gf_ftp_download_to_mem(ftp_url, &size, "goeful", "goeful", 1000);
	if(mem != NULL)
	{
		free(mem);
		mem = NULL;
	}
#endif
	gf_ftp_upload_from_file(ftp_url, localname, "goeful", "goeful", 1000);

    return 0;  
}  
