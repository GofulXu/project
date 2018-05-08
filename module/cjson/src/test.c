#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "parse.h"
#define LOGIN_STATUS 1

int main(int argc, char *argv[])
{
	char *CC;
	Login_msg m, mm;
	cJSON *root = cJSON_CreateObject();
	strcpy(m.url, "tcp:192.168.1.108:9999");
	m.connect_type = 2;
	m.devtype = 440240;
	strcpy(m.username, "goeful");
	m.devid = 601440240;
	strcpy(m.password, "goeful123..");
	m.cmdid = LOGIN_STATUS;
	strcpy(m.cmdmsg, "login");
	create_json_login(root, &m);
	CC = cJSON_PrintUnformatted(root);
	printf("cc:%s\n", CC);
	cJSON *head = cJSON_Parse(CC);
	doit_login(head, &mm);
	printf("json array:%d\n", cJSON_GetArraySize(root));
	printf("head array:%d\n", cJSON_GetArraySize(head));
	free(CC);
	cJSON_Delete(root);
	cJSON_Delete(head);
	printfmsg(&mm);
	return 0;
}
