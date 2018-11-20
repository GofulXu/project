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
	{"volume", "10"},

	{"lan_ip", "10.10.10.10"},
	{"lan_mask", "11.11.11.11"},
	{"lan_gateway", "12.12.12.12"},

	{"wan_ip", "20.20.20.20"},
	{"wan_mask", "21.21.21.21"},
	{"wan_gateway", "22.22.22.22"},

	{"net_dns1", "172.16.18.18"},
	{"net_dns2", "114.114.114.114"},

	{"net_eth0_netmask", "255.255.255.0"},
	{"net_eth1_netmask", "255.255.255.0"},
	{"net_eth2_netmask", "255.255.255.0"},
	{"net_eth3_netmask", "255.255.255.0"},

	{"net_eth0_gateway", "192.168.1.1"},
	{"net_eth1_gateway", "192.168.2.1"},
	{"net_eth2_gateway", "192.168.3.1"},
	{"net_eth3_gateway", "192.168.4.1"},

	{"net_eth0_ipaddr", "192.168.1.242"},
	{"net_eth1_ipaddr", "192.168.2.242"},
	{"net_eth2_ipaddr", "192.168.3.242"},
	{"net_eth3_ipaddr", "192.168.4.242"}
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

	db = database_open(PARAM_PATH);
	printf("\n\t%s\t\n", SYSTEM_PARA_TABLE);
	database_show(db, SYSTEM_PARA_TABLE);
	printf("\n\t%s\t\n", GENERAL_PARA_TABLE);
	database_show(db, GENERAL_PARA_TABLE);
	database_close(db);
	return 0;
}
