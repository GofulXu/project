#include "gfapi.h"
#include "sqlite3/operate.h"
#include "gfparamerter.h"

#define VALUE_SIZE	256
static sqlite3 *db = NULL;

typedef struct _mpara_t{
	char name[VALUE_SIZE];
	char value[VALUE_SIZE];
}mpara_t;


mpara_t system_nu[] = {
	{"goeful1", "111"},
	{"goeful2", "222"},
	{"goeful3", "333"}
};

mpara_t general_nu[] = {
	{"goeful4", "444"},
	{"goeful5", "555"},
	{"goeful6", "666"}
};

int main(int argc, char *argv[])
{

	db = database_open(PARAM_PATH);
	chmod(PARAM_PATH, 0775);

	database_general_create_paratable(db, SYSTEM_PARA_TABLE);
	database_general_create_paratable(db, GENERAL_PARA_TABLE);

	database_close(db);

	gf_paramerter_init();

	int i = 0;
	for(i = 0; i < sizeof(system_nu)/(2*VALUE_SIZE); i++)
		gf_paramerter_insert_system(system_nu[i].name, general_nu[i].value);

	for(i = 0; i < sizeof(general_nu)/(2*VALUE_SIZE); i++)
		gf_paramerter_insert(general_nu[i].name, general_nu[i].value);

	char sub[256];


	memset(sub, 0, sizeof(sub));
	gf_paramerter_get("goeful5", sub, sizeof(sub));
	printf("get goeful5:%s\n", sub);
	printf("get int goeful5:%d\n", gf_paramerter_get_int("goeful5"));

	memset(sub, 0, sizeof(sub));
	gf_paramerter_get("goeful1", sub, sizeof(sub));
	printf("get goeful1:%s\n", sub);
	printf("get int goeful1:%d\n", gf_paramerter_get_int("goeful1"));


	gf_paramerter_set("goeful5", "1234");
	gf_paramerter_set("goeful1", "5678");

	memset(sub, 0, sizeof(sub));
	gf_paramerter_get("goeful5", sub, sizeof(sub));
	printf("get goeful5:%s\n", sub);
	printf("get int goeful5:%d\n", gf_paramerter_get_int("goeful5"));

	memset(sub, 0, sizeof(sub));
	gf_paramerter_get("goeful1", sub, sizeof(sub));
	printf("get goeful1:%s\n", sub);
	printf("get int goeful1:%d\n", gf_paramerter_get_int("goeful1"));

	gf_paramerter_set_int("goeful5", 1111);
	gf_paramerter_set_int("goeful1", 2222);

	memset(sub, 0, sizeof(sub));
	gf_paramerter_get("goeful5", sub, sizeof(sub));
	printf("get goeful5:%s\n", sub);
	printf("get int goeful5:%d\n", gf_paramerter_get_int("goeful5"));

	memset(sub, 0, sizeof(sub));
	gf_paramerter_get("goeful1", sub, sizeof(sub));
	printf("get goeful1:%s\n", sub);
	printf("get int goeful1:%d\n", gf_paramerter_get_int("goeful1"));

	gf_paramerter_exit();

	db = database_open(PARAM_PATH);
	printf("\n\tsystem\t\n");
	database_show(db, SYSTEM_PARA_TABLE);
	printf("\n\tgeneral\t\n");
	database_show(db, GENERAL_PARA_TABLE);
	database_close(db);

	return 0;
}
