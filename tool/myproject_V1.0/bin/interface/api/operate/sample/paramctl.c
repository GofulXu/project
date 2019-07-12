#include "gfapi.h"
#include "gfthrd.h"
#include "operate.h"

#if 0
int main(int argc, char *argv[])
{
	sqlite3 *db = database_open(SQLITE3_DBPATH);
	database_general_create_paratable(db, SYSTEM_PARA_TABLE);
	char sub[256] = {0};
	int now = gf_thrd_get_tick();

	int i = 0;
	for(i = 0; i < 10000; i++)
		database_insert_system_paratable(db, SYSTEM_PARA_TABLE, "goeful", "goeful");

	printf("insert 10000 paramerter time:%ds\n", (gf_thrd_get_tick() - now) /1000);

	database_insert_system_paratable(db, SYSTEM_PARA_TABLE, "bin", "bin");

	now = gf_thrd_get_tick();
	for(i = 0; i < 10000; i++)
		database_insert_system_paratable(db, SYSTEM_PARA_TABLE, "goeful", "goeful");

	printf("insert 20000 paramerter time:%ds\n", (gf_thrd_get_tick() - now) /1000);

	now = gf_thrd_get_tick();
	database_general_query(db, SYSTEM_PARA_TABLE, "bin", sub, sizeof(sub));
	printf("find param:%s-->%s in 2000 paramerter time:%ds\n", "bin", sub, (gf_thrd_get_tick() - now) /1000);

//	database_show(db, SYSTEM_PARA_TABLE);
	database_close(db);
	return 0;
}
#else
int main(int argc, char *argv[])
{
	sqlite3 *db = database_open(SQLITE3_DBPATH);
	if(argc == 2 && strstr(argv[1], "help"))
	{
		printf("***************************\n");
		printf("*** -C to create table = argv[2] in sqlite\tsystem or secure\n");
		printf("*** -D to delete name = argv[2] in sqlite\n");
		printf("*** -U to update name = argv[2] value = argv[3] in sqlite\n");
		printf("*** -I to insert name = argv[2] value = argv[3] in sqlite\n");
		printf("*** -G to get name = argv[2] in sqlite value\n");
		printf("*** -S to show obj = argv[2] in sqlite\n");
		printf("*** default obj in sqlite about secure and system\n");
	}else if(argc == 1)
	{
		database_show(db, SYSTEM_PARA_TABLE);														\
		database_show(db, GENERAL_PARA_TABLE);														\
		database_close(db);
		return 0;
	}

	int ret = -5;
	char sub[128];
	memset(sub, 0, sizeof(sub));
	switch(argv[1][1])
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
					if(database_general_query(db, SYSTEM_PARA_TABLE, argv[2], sub, sizeof(sub)) == QUERY_OK)			\
						database_delete(db, SYSTEM_PARA_TABLE, argv[2]);										\
					else if(database_general_query(db, GENERAL_PARA_TABLE, argv[2], sub, sizeof(sub)) == QUERY_OK)		\
						database_delete(db, GENERAL_PARA_TABLE, argv[2]);										\
					else																						\
					{																							\
						printf("del->%s err no found\n", argv[2]);												\
						break;																					\
					}																							\
					printf("del->%s suc\n", argv[2]);															\
					break;
		case 'I':																								\
					if(DB_OK != database_general_insert_paratable(db, GENERAL_PARA_TABLE, argv[2], argv[3]))	\
					{																							\
						printf("insert:%s->%s err\n", argv[2], argv[3]);										\
						break;																					\
					}																							\
					printf("insert:%s->%s suc\n", argv[2], argv[3]);											\
					break;

		case 'U':																								\
					if(database_general_query(db, SYSTEM_PARA_TABLE, argv[2], sub, sizeof(sub)) == QUERY_OK)	\
						database_general_update(db, SYSTEM_PARA_TABLE, argv[2], argv[3]);
					else if(database_general_query(db, GENERAL_PARA_TABLE, argv[2], sub, sizeof(sub)) == QUERY_OK)	\
						database_general_update(db, GENERAL_PARA_TABLE, argv[2], argv[3]);
					else																						\
					{																							\
						printf("update %s err no found\n", argv[2]);											\
						break;																					\
					}																							\
					printf("update:%s->%s\n", argv[2], argv[3]);												\
					break;
		case 'G':																								\
					if(database_general_query(db, SYSTEM_PARA_TABLE, argv[2], sub, sizeof(sub)) == QUERY_OK);	\
					else if(database_general_query(db, GENERAL_PARA_TABLE, argv[2], sub, sizeof(sub)) == QUERY_OK);	\
					else																						\
					{																							\
						printf("get %s err no found\n", argv[2]);												\
						break;																					\
					}																							\
					printf("get %s->%s\n", argv[2], sub);														\
					break;	
		case 'S':																								\
					database_show(db, argv[2]);																	\
					break;
		default:																								\
					break;
	}

	database_close(db);
	return 0;
}
#endif
