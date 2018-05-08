#ifndef __PARSE_H__
#define __PARSE_H__
#include "cJSON.h"


enum _usercmd{
	LOGIN_STATUS = 10,
	GET_DEV_STATUS,
	GET_DEV_VALUE,
	SET_DEV_CMD
};

enum _devcmd{
	RESPONT_DEV_VALUE = 1000,
};

typedef struct _Login_msg{
	char url[64];
	int connect_type;
	int devtype;
	char username[32];
	int devid;
	int userid;
	char password[32];
	int cmdid;
	char cmdmsg[128];
}Login_msg;

int printfmsg(Login_msg *m);


int create_json_login(cJSON *root, const Login_msg *m);

int doit_login(cJSON *root, Login_msg *m);

#endif /*__PARSE_H__*/
