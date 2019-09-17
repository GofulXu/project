#ifndef __GFOPERATE_H__
#define __GFOPERATE_H__

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

#define GENERAL_PARA_TABLE	"ParaInfo"
#define GENERAL_DEV_TABLE	"DevInfo"

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

sqlite3 *DatabaseOpen(char *path);
int DatabaseClose(sqlite3 *Db);
int DatabaseDeleteParaTable(sqlite3 *Db, char *Table, char *Name);
int DatabaseGeneralCreateParaTable(sqlite3* Db, char *Table);
int DatabaseGeneralInsertParaTable(sqlite3* Db, char *Table, char *Name, char *Value);
int DatabaseShow(sqlite3 *Db, char *Table);
int DatabaseGeneralUpdateParaTable(sqlite3* Db, char *Table, char *Name, char *Value);
int DatabaseGeneralQueryParaTable(sqlite3* Db, char *Table, char *Name, char *Value, int Size);

int DatabaseGeneralCreateDevTable(sqlite3* Db, char *Table);
int DatabaseGeneralInsertDevTable(sqlite3* Db, char *Table, char *Uuid, char *UserID, char *Passwd, char *UserName, unsigned int Type);
int DatabaseGeneralUpdateDevTable(sqlite3* Db, char *Table, char *Uuid, char *Passwd, char *UserName);
int DatabaseGeneralQueryDevTable(sqlite3* Db, char *Table, char *Uuid, char *UserId, int IdSize, char *Passwd, int WdSize, char *UserName, int NaSize, int *Type);
int DatabaseDeleteDevTable(sqlite3 *Db, char *Table, char *Uuid);
#endif /*__GFOPERATE_H__*/

