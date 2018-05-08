#ifndef __PARSE_H__
#define __PARSE_H__
#include "cJSON.h"
#include "uthash.h"

typedef struct _Login_msg{
	int sockfd;
	int dev_status;
	char url[64];
	int connect_type;
	int devtype;
	char username[32];
	int userid;
	int devid;
	char password[32];
	int cmdid;
	char cmdmsg[128];
    UT_hash_handle hh;
}Login_msg;

int printfmsg(Login_msg *m);


int create_json_login(cJSON *root, const Login_msg *m);

int doit_login(cJSON *root, Login_msg *m);

#endif /*__PARSE_H__*/
