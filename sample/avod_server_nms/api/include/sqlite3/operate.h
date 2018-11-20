#ifndef __OPERATE_H__
#define __OPERATE_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "sqlite3.h"

#ifdef __cplusplus
}
#endif

#define LOG_DIR "/mnt/sdcard/logs"

#define SQLITE3_DBPATH	"/usr/.msqlite3.db"

#define SYSTEM_PARA_TABLE	"system"
#define GENERAL_PARA_TABLE	"secure"

typedef struct _general_param{
		long long id;
		char name[64];
		char value[256];
}general_param_t;

typedef struct _system_param{
		long long id;
		int significant;
		char name[64];
		char value[256];
}system_param_t;



enum {
	DB_ERROR = -4,
	DB_NO_FOUND = -3,
	QUERY_ERROR = -2,
	QUERY_NO_FOUND = -1,
	QUERY_OK = 0,
	DB_OK = 0
};

sqlite3 *database_open(char *path);

int database_close(sqlite3 *db);

int database_delete(sqlite3 *db, char *table, char *name);

int database_general_create_paratable(sqlite3* db, char *table);

int database_general_insert_paratable(sqlite3* db, char *table, char *name, char *value);

int database_show(sqlite3 *db, char *table);

int database_general_update(sqlite3* db, char *table, char *name, char *value);

int database_general_query(sqlite3* db, char *table, char *name, char *value, int size);
#endif /*__OPERATE_H__*/

