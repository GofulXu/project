#include "default/gfapi.h"
#include "cJSON.h"

static int create_json_str(char *str, char *valuestring, unsigned int valueint)
{
	if(NULL != str && NULL != valuestring)
		return sprintf(str, "{name:\"%s\", size:\"%d\"}", valuestring, valueint);
	return -1;
}

static int get_json_value(const char *str, char *valuestring,  unsigned int *valueint)
{
	int i = 0, j = 0;
	int suc = 0;
	char num[32];
	for(i = 0; i < strlen(str); i++)
	{
		if(str[i] == 's' && str[i+1] == 'i' && str[i+2] == 'z' && str[i+3] == 'e' && str[i+4] == ':')
		{
			i += 6;
			for(j = i; j < strlen(str); j ++)
			{
				if(str[j] == '"')
				{
					memset(num, 0, j-i);
					strncpy(num, str+i, j-i);
					*valueint = atoi(num);
					suc = !suc;
					break;
				}
			}
		}
		if(str[i] == 'n' && str[i+1] == 'a' && str[i+2] == 'm' && str[i+3] == 'e' && str[i+4] == ':')
		{
			i += 6;
			for(j = i; j < strlen(str); j ++)
			{
				if(str[j] == '"')
				{
					memset(valuestring, 0, j-i);
					strncpy(valuestring, str+i, j-i);
					suc = !suc;
					break;
				}
			}
		}
	}
	return suc;
}

#if 0
int main(int argc, char *argv[])
{
	char buf[128];
	unsigned int size;
	char name[64];
	memset(buf, 0, sizeof(buf));
	create_json_str(buf, "xuwenwang", 1024*2014*10);
	if(!get_json_value(buf, name, &size))
		printf("get value suc name:%s\tsize:%d\n", name, size);
	return 0;

}

#else

int main(int argc, char *argv[])
{
	char buf[128];
	int fd;
	struct stat mst;
	if(argc < 3)
	{
		printf("argc:%d\n", argc);
		return 0;
	}
	if(stat(argv[1], &mst))
	{
		printf("argv1:%s\n", argv[1]);
		return 0;
	}

	fd = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0775);
	if(fd > 0)
	{
		memset(buf, 0, sizeof(buf));
		create_json_str(buf, argv[1], mst.st_size);
		write(fd, buf, strlen(buf));
		close(fd);
		fd = -1;
	}

#if 0
	fd = open(argv[1], O_RDONLY);
	if(fd > 0)
	{
		memset(buf, 0, sizeof(buf));
		read(fd, buf, sizeof(buf));
		printf("buf:%s", buf);
		close(fd);
		fd = -1;
	}

	cJSON *head = cJSON_Parse(buf);
	if(head != NULL)
	{
		printf("filename:%s\tsize:%d", cJSON_GetObjectItem(head, "name")->valuestring, cJSON_GetObjectItem(head, "size")->valueint);
		cJSON_Delete(head);
	}
#endif
	return 0;
}
#endif
