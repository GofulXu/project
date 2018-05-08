#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include "mbase64.h"

int main(int argc, char *argv[])
{
	char endata[1024] = {0};
	char dedata[1024] = {0};
	int len = 0;
	FILE *in_fp = NULL;
	FILE *out_fp = NULL;
	if(argc > 2)
	{
		if(strcmp(argv[3], "-d") == 0){
		in_fp = fopen(argv[1],"r");
		out_fp = fopen(argv[2], "w+");
		decode(in_fp, out_fp);
		printf("decode %s--->%s success\n", argv[1], argv[2]);
		}else
		{
		in_fp = fopen(argv[1],"r");
		out_fp = fopen(argv[2], "w+");
		encode(in_fp, out_fp);
		printf("encode %s--->%s success\n", argv[1], argv[2]);
		}
		if(in_fp)fclose(in_fp);
		if(out_fp)fclose(out_fp);
		return 0;

	}

	
	printf("please input to encode: ");
	memset(endata,'0',sizeof(endata));
	memset(dedata,'0', sizeof(dedata));
	//scanf("%s", endata);
	strcpy(endata, "{\"scene\":\"main\", \"userid\":\"user_0001\"}");
	base64_encode(endata,  strlen(endata), dedata);
	printf("endata:%s\ndedata:%s\n",endata, dedata);

	printf("please input to decode: ");
	memset(endata,'0',sizeof(endata));
	memset(dedata,'0', sizeof(dedata));
	scanf("%s", endata);
	len = base64_decode(endata, strlen(endata), dedata);
	printf("endata:%s\ndedata:%s\nlen:%d\n",endata, dedata, len);
	return 0;
}

