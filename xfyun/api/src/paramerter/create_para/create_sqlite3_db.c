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
	{"goeful3", "333"},
	{"goeful4", "444"},
	{"goeful5", "555"},
	{"goeful6", "666"}
};

mpara_t general_nu[] = {
	{"log_type", "0"},
	{"log_level", "0"},
	{"log_targets", "console"},
	{"lan_ip", "192.168.1.108"},
	{"wan_ip", "192.168.0.108"},
	{"udp_hostip", "192.168.1.242"},
	{"udp_hostport", "9999"},
	{"udp_UdpRecvTimeout", "600000"},
	{"udp_UdpHeartTimeout", "1000"}
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
		gf_paramerter_insert_system(system_nu[i].name, system_nu[i].value);

	for(i = 0; i < sizeof(general_nu)/(2*VALUE_SIZE); i++)
		gf_paramerter_insert(general_nu[i].name, general_nu[i].value);

	gf_paramerter_exit();

#if defined(SUPPORT_DEBUG_MODE1) || defined(SUPPORT_DEBUG_MODE2) || defined(SUPPORT_DEBUG_MODE3) || defined(SUPPORT_DEBUG_MODE)
	db = database_open(PARAM_PATH);
	printf("\n\tsystem\t\n");
	database_show(db, SYSTEM_PARA_TABLE);
	printf("\n\tgeneral\t\n");
	database_show(db, GENERAL_PARA_TABLE);
	database_close(db);
#endif
	return 0;
}
