#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "cJSON.h"

int printfmsg(Login_msg *m)
{
	if(NULL != m->url)
		printf("url:%s\n", m->url);
	if(0 != m->connect_type)
		printf("connect_type:%d\n", m->connect_type);
	if(0 != m->devtype)
		printf("devtype:%d\n", m->devtype);
	if(NULL != m->username)
		printf("username:%s\n", m->username);
	if(0 != m->devid)
		printf("devid:%d\n", m->devid);
	if(NULL != m->password)
		printf("password:%s\n", m->password);
	if(0 != m->cmdid)
		printf("cmdid:%d\n", m->cmdid);
	if(NULL != m->cmdmsg)
		printf("cmdmsg:%s\n", m->cmdmsg);

	return 0;
}

int create_json_login(cJSON *root, const Login_msg *m)
{
	if(NULL == m || NULL == root)
		return -1;
	cJSON *users, *cmds;
	users = cJSON_CreateObject();
	cmds = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "url", m->url);
	cJSON_AddNumberToObject(root, "connect_type", m->connect_type);
	cJSON_AddNumberToObject(users, "devtype", m->devtype);
	cJSON_AddStringToObject(users, "name", m->username);
	cJSON_AddNumberToObject(users, "devid", m->devid);
	cJSON_AddNumberToObject(users, "userid", m->userid);
	cJSON_AddStringToObject(users, "password", m->password);
	cJSON_AddNumberToObject(cmds, "cmdid", m->cmdid);
	cJSON_AddStringToObject(cmds, "cmdmsg", m->cmdmsg);
	cJSON_AddItemToObjectCS(root,"devs", users);
	cJSON_AddItemToObjectCS(root,"cmds", cmds);
	return 0;
}

int doit_login(cJSON *root, Login_msg *m)
{
	if(cJSON_GetArraySize(root) < 4)
		return -1;
	cJSON *devs = cJSON_GetObjectItem(root, "devs");
	if(NULL == devs)
		return -1;
	cJSON *cmds = cJSON_GetObjectItem(root, "cmds");
	if(NULL == cmds)
		return -1;
	if(cJSON_GetArraySize(devs) < 2 || cJSON_GetArraySize(cmds) < 2)
		return -1;

	memset(m, 0, sizeof(Login_msg));
	strncpy(m->url, cJSON_GetObjectItem(root, "url")->valuestring, sizeof(m->url));
	m->connect_type = cJSON_GetObjectItem(root, "connect_type")->valueint;
	m->devtype = cJSON_GetObjectItem(devs, "devtype")->valueint;
	strncpy(m->username, cJSON_GetObjectItem(devs, "name")->valuestring, sizeof(m->url));
	m->devid = cJSON_GetObjectItem(devs, "devid")->valueint;
	m->userid = cJSON_GetObjectItem(devs, "userid")->valueint;
	strncpy(m->password, cJSON_GetObjectItem(devs, "password")->valuestring, sizeof(m->url));
	m->cmdid = cJSON_GetObjectItem(cmds, "cmdid")->valueint;
	strncpy(m->cmdmsg, cJSON_GetObjectItem(cmds, "cmdmsg")->valuestring, sizeof(m->cmdmsg));


	return 0;
}
#if 0

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
#endif
