#include "gfapi.h"
#include "sqlite3/GfOperate.h"
#include "GfParamerter.h"

static sqlite3 *Db = NULL;

typedef struct _ParaInfo{
	char Name[256];
	char Value[256];
}ParaInfo;

typedef struct _DevInfo{
    char Uuid[32];
    char UserId[32];
    char Passwd[32];
    char UserName[32];
    int Type;
}DevInfo;

DevInfo InfoDev[] = {
    {"10000", "10001", "10002", "10003", 101},
    {"20000", "20001", "20002", "20003", 201},
    {"30000", "30001", "30002", "30003", 301},
    {"40000", "40001", "40002", "40003", 401},
    {"50000", "50001", "50002", "50003", 501},
};

ParaInfo InfoPara[] = {
	{"log_type", "0"},
	{"log_level", "0"},
	{"log_targets", "console"},
	{"lan_ip", "192.168.1.108"},
	{"wan_ip", "192.168.0.108"},
	{"udp_hostip", "192.168.1.242"},
	{"udp_hostport", "9999"},
	{"udp_UdpRecvTimeout", "600000"},
	{"udp_UdpHeartTimeout", "1000"},
};

int main(int argc, char *argv[])
{
	int i = 0;

	Db = DatabaseOpen(PARAM_PATH);

	chmod(PARAM_PATH, 0775);

	DatabaseGeneralCreateParaTable(Db, GENERAL_PARA_TABLE);
	DatabaseGeneralCreateDevTable(Db, GENERAL_DEV_TABLE);

	for(i = 0; i < sizeof(InfoDev)/sizeof(DevInfo); i++)
	    DatabaseGeneralInsertDevTable(Db, GENERAL_DEV_TABLE, InfoDev[i].Uuid, InfoDev[i].UserId, InfoDev[i].Passwd, InfoDev[i].UserName, InfoDev[i].Type);


	for(i = 0; i < sizeof(InfoPara)/sizeof(ParaInfo); i++)
		DatabaseGeneralInsertParaTable(Db, GENERAL_PARA_TABLE, InfoPara[i].Name, InfoPara[i].Value);


	printf("\n\t%s\t\n", GENERAL_PARA_TABLE);
	DatabaseShow(Db, GENERAL_PARA_TABLE);
	printf("\n\t%s\t\n", GENERAL_DEV_TABLE);
	DatabaseShow(Db, GENERAL_DEV_TABLE);
	DatabaseClose(Db);
	return 0;
}
