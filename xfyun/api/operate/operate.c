#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/statfs.h>
#include <dirent.h>
#include "stdafx.h"
#include "operate.h"

sqlite3 *database_open(char *path)
{
	sqlite3 *db = NULL;
	int result = -1;
	if(!path)
		result = sqlite3_open(SQLITE3_DBPATH, &db);
	else
		result = sqlite3_open(path, &db);
		
	if(result == SQLITE_OK)
		return db;
	else
		return NULL;
}

int database_close(sqlite3 *db)
{
	if(!db)
		return DB_NO_FOUND;
	int result = sqlite3_close(db);
	if(result != SQLITE_OK)
		return DB_ERROR;
	return DB_OK;
}


int database_delete(sqlite3 *db, char *table, char *name)
{
	if(!db)
		return DB_NO_FOUND;
	char *errmsg = NULL;
	char sql_sentence[256];
	memset(sql_sentence, 0, sizeof(sql_sentence));
	snprintf(sql_sentence, sizeof(sql_sentence), "DELETE FROM %s WHERE name = '%s';", table, name); 

	int result = sqlite3_exec(db, sql_sentence, 0, 0, &errmsg);
	if(result != SQLITE_OK)
		return DB_ERROR;
	return DB_OK;	
}

int database_show(sqlite3 *db, char *table)
{
	if(!db)
		return DB_NO_FOUND;
	char ** resultp;
	char *errmsg;
	int nrow,ncolumn,i,j,index;
	char sql[256] = {0};
	snprintf(sql, sizeof(sql), "select * from %s", table);

	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncolumn,&errmsg) != SQLITE_OK)
	{
		perror("sqlite3_get_table");
		return DB_ERROR;
	}

	index = ncolumn;
	for(i = 0 ;i< ncolumn; i++)
		printf("%s\t",resultp[i]);
	printf("\n");

	for(i = 0;i< nrow;i++)
	{
		for(j = 0;j< ncolumn;j++)
			printf("%s\t",resultp[index++]);
		printf("\n");
	}
	return DB_OK;
}

int database_general_create_paratable(sqlite3* db, char *table)
{
	if(!db)
		return DB_NO_FOUND;
	char *errmsg = NULL;
	char sql_sentence[256];
	memset(sql_sentence, 0, sizeof(sql_sentence));
	snprintf(sql_sentence, sizeof(sql_sentence), "	\
		CREATE TABLE %s(							\
		ID INTEGER PRIMARY KEY,						\
		NAME TEXT,									\
		VALUE TEXT									\
	  );", table); 

	int result = sqlite3_exec(db, sql_sentence, 0, 0, &errmsg);
	if(result != SQLITE_OK)
		return DB_ERROR;
	return DB_OK;	
}

int database_general_insert_paratable(sqlite3* db, char *table, char *name, char *value)
{
	if(!db)
		return DB_NO_FOUND;
	char *errmsg = NULL;
	char sql_sentence[256];
	memset(sql_sentence, 0, sizeof(sql_sentence));
	snprintf(sql_sentence, sizeof(sql_sentence), "INSERT INTO \"%s\" VALUES(NULL, '%s', '%s');", table, name, value); 
	int result = sqlite3_exec(db, sql_sentence, 0, 0, &errmsg);
	if(result != SQLITE_OK)
		return DB_ERROR;
	return DB_OK;	
}


int database_general_update(sqlite3* db, char *table, char *name, char *value)
{
	if(!db)
		return DB_NO_FOUND;
	char sub[128] = {0};
	if(QUERY_OK != database_general_query(db, table, name, sub, sizeof(sub)))
		return QUERY_NO_FOUND;
	char *errmsg = NULL;
	char sql_sentence[256];
	memset(sql_sentence, 0, sizeof(sql_sentence));
	snprintf(sql_sentence, sizeof(sql_sentence), "update %s set value='%s' where name='%s';", table, value, name); 

	int result = sqlite3_exec(db, sql_sentence, 0, 0, &errmsg);
	if(result != SQLITE_OK)
		return DB_ERROR;
	return DB_OK;	
}

int database_general_query(sqlite3* db, char *table, char *name, char *value, int size)
{
	if(!db)
		return DB_NO_FOUND;
	char **dbResult;
	int nRow, nColumn;
	char * errmsg = NULL;
	int i, j, index, result;
	char sql_sentence[256];
	memset(sql_sentence, 0, sizeof(sql_sentence));
	snprintf(sql_sentence, sizeof(sql_sentence), "select * from %s where name='%s';", table, name); 

	result = sqlite3_get_table( db, sql_sentence, &dbResult, &nRow, &nColumn, &errmsg);
	if(result == SQLITE_OK)
	{
		if(nRow > 0 && nColumn > 0)
		{
			index = nColumn;
			for(i = 0; i < nRow; i ++)
			{
				for(j = 0; j < nColumn; j ++)		
				{
					//printf("[query]name:%d:%s, value:%d:%s\n", i,dbResult[j], j,dbResult[index]);
					if(j == 2)
						snprintf(value, size, "%s", dbResult[index]);
					index ++;
				}
			}
		}
		else
		{
			sqlite3_free_table(dbResult);
			return QUERY_NO_FOUND;
		}

	}
	else
		return QUERY_ERROR;	
	sqlite3_free_table(dbResult);
	return QUERY_OK;
}
