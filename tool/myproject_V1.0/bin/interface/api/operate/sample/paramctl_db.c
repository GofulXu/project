#include "gfapi.h"
#include "gfthrd.h"
#include "operate.h"

int main(int argc, char *argv[])
{
	if(argc == 2 && strstr(argv[1], "help"))
	{
		printf("***************************\n");
		printf("*** db -C to create table = argv[2] in sqlite\tsystem or secure\n");
		printf("*** db -D to delete name = argv[2] in sqlite\n");
		printf("*** db -U to update name = argv[2] value = argv[3] in sqlite\n");
		printf("*** db -I to insert name = argv[2] value = argv[3] in sqlite\n");
		printf("*** db -G to get name = argv[2] in sqlite value\n");
		printf("*** db -S to show obj = argv[2] in sqlite\n");
		printf("*** default obj in sqlite about secure and system\n");
		return 0;
	}else if(argc <= 2)
	{
		sqlite3 *db = database_open(SQLITE3_DBPATH);
		database_show(db, SYSTEM_PARA_TABLE);														\
		database_show(db, GENERAL_PARA_TABLE);														\
		database_close(db);
		return 0;
	}
	sqlite3 *db = database_open(argv[1]);

	int ret = -5;
	char sub[128];
	memset(sub, 0, sizeof(sub));
	switch(argv[2][1])
	{
		case 'C':																								\
					if(database_general_create_paratable(db, SYSTEM_PARA_TABLE) != DB_OK || database_general_create_paratable(db, GENERAL_PARA_TABLE) != DB_OK)																		\
					{																							\
						printf("create table %s|%s err\n", SYSTEM_PARA_TABLE, GENERAL_PARA_TABLE);				\
						break;																					\
					}																							\
					printf("create table %s|%s suc\n", SYSTEM_PARA_TABLE, GENERAL_PARA_TABLE);					\
					break;
					
		case 'D':																								\
					if(database_general_query(db, SYSTEM_PARA_TABLE, argv[3], sub, sizeof(sub)) == QUERY_OK)			\
						database_delete(db, SYSTEM_PARA_TABLE, argv[3]);										\
					else if(database_general_query(db, GENERAL_PARA_TABLE, argv[3], sub, sizeof(sub)) == QUERY_OK)		\
						database_delete(db, GENERAL_PARA_TABLE, argv[3]);										\
					else																						\
					{																							\
						printf("del->%s err no found\n", argv[3]);												\
						break;																					\
					}																							\
					printf("del->%s suc\n", argv[3]);															\
					break;
		case 'I':																								\
					if(DB_OK != database_general_insert_paratable(db, GENERAL_PARA_TABLE, argv[3], argv[4]))	\
					{																							\
						printf("insert:%s->%s err\n", argv[3], argv[4]);										\
						break;																					\
					}																							\
					printf("insert:%s->%s suc\n", argv[3], argv[4]);											\
					break;

		case 'U':																								\
					if(database_general_query(db, SYSTEM_PARA_TABLE, argv[3], sub, sizeof(sub)) == QUERY_OK)	\
						database_general_update(db, SYSTEM_PARA_TABLE, argv[3], argv[4]);
					else if(database_general_query(db, GENERAL_PARA_TABLE, argv[3], sub, sizeof(sub)) == QUERY_OK)	\
						database_general_update(db, GENERAL_PARA_TABLE, argv[3], argv[4]);
					else																						\
					{																							\
						printf("update %s err no found\n", argv[3]);											\
						break;																					\
					}																							\
					printf("update:%s->%s\n", argv[3], argv[4]);												\
					break;
		case 'G':																								\
					if(database_general_query(db, SYSTEM_PARA_TABLE, argv[3], sub, sizeof(sub)) == QUERY_OK);	\
					else if(database_general_query(db, GENERAL_PARA_TABLE, argv[3], sub, sizeof(sub)) == QUERY_OK);	\
					else																						\
					{																							\
						printf("get %s err no found\n", argv[3]);												\
						break;																					\
					}																							\
					printf("get %s->%s\n", argv[3], sub);														\
					break;	
		case 'S':																								\
					if(argc == 4)																				\
						database_show(db, argv[3]);																\
					else																						\
					{																							\
						printf("show table %s\n", GENERAL_PARA_TABLE);											\
						database_show(db,GENERAL_PARA_TABLE);													\
						printf("show table %s\n", SYSTEM_PARA_TABLE);											\
						database_show(db,SYSTEM_PARA_TABLE);													\
					}																							\
					break;
		default:																								\
					break;
	}

	database_close(db);
	return 0;
}
