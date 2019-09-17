#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/statfs.h>
#include <dirent.h>
#include "stdafx.h"
#include "GfOperate.h"

sqlite3 *DatabaseOpen(char *path)
{
	sqlite3 *Db = NULL;
	int Result = -1;
	if(!path)
		Result = sqlite3_open(SQLITE3_DBPATH, &Db);
	else
		Result = sqlite3_open(path, &Db);
		
	if(Result == SQLITE_OK)
		return Db;
	else
		return NULL;
}

int DatabaseClose(sqlite3 *Db)
{
	if(!Db)
		return DB_NO_FOUND;
	int Result = sqlite3_close(Db);
	if(Result != SQLITE_OK)
		return DB_ERROR;
	return DB_OK;
}


int DatabaseShow(sqlite3 *Db, char *Table)
{
	if(!Db)
		return DB_NO_FOUND;
	char ** Resultp;
	char *Errmsg;
	int nrow,ncolumn,i,j,index;
	char sql[256] = {0};
	snprintf(sql, sizeof(sql), "select * from %s", Table);

	if(sqlite3_get_table(Db,sql,&Resultp,&nrow,&ncolumn,&Errmsg) != SQLITE_OK)
	{
		perror("sqlite3_get_table");
		return DB_ERROR;
	}

	index = ncolumn;
	for(i = 0 ;i< ncolumn; i++)
		printf("%s\t",Resultp[i]);
	printf("\n");

	for(i = 0;i< nrow;i++)
	{
		for(j = 0;j< ncolumn;j++)
			printf("%s\t",Resultp[index++]);
		printf("\n");
	}
	return DB_OK;
}

int DatabaseGeneralCreateParaTable(sqlite3* Db, char *Table)
{
	if(!Db)
		return DB_NO_FOUND;
	char *Errmsg = NULL;
	char SqlSentence[256];
	memset(SqlSentence, 0, sizeof(SqlSentence));
	snprintf(SqlSentence, sizeof(SqlSentence), "	\
		CREATE Table %s(							\
		ID INTEGER PRIMARY KEY,						\
		Name TEXT,									\
		Value TEXT									\
	  );", Table); 

	int Result = sqlite3_exec(Db, SqlSentence, 0, 0, &Errmsg);
	if(Result != SQLITE_OK)
		return DB_ERROR;
	return DB_OK;	
}

int DatabaseGeneralInsertParaTable(sqlite3* Db, char *Table, char *Name, char *Value)
{
	if(!Db)
		return DB_NO_FOUND;
	char *Errmsg = NULL;
	char SqlSentence[256];
	memset(SqlSentence, 0, sizeof(SqlSentence));
	snprintf(SqlSentence, sizeof(SqlSentence), "INSERT INTO \"%s\" ValueS(NULL, '%s', '%s');", Table, Name, Value); 
	int Result = sqlite3_exec(Db, SqlSentence, 0, 0, &Errmsg);
	if(Result != SQLITE_OK)
		return DB_ERROR;
	return DB_OK;	
}


int DatabaseGeneralUpdateParaTable(sqlite3* Db, char *Table, char *Name, char *Value)
{
	if(!Db)
		return DB_NO_FOUND;
	char sub[128] = {0};
	if(QUERY_OK != DatabaseGeneralQueryParaTable(Db, Table, Name, sub, sizeof(sub)))
		return QUERY_NO_FOUND;
	char *Errmsg = NULL;
	char SqlSentence[256];
	memset(SqlSentence, 0, sizeof(SqlSentence));
	snprintf(SqlSentence, sizeof(SqlSentence), "update %s set Value='%s' where Name='%s';", Table, Value, Name); 

	int Result = sqlite3_exec(Db, SqlSentence, 0, 0, &Errmsg);
	if(Result != SQLITE_OK)
		return DB_ERROR;
	return DB_OK;	
}

int DatabaseGeneralQueryParaTable(sqlite3* Db, char *Table, char *Name, char *Value, int Size)
{
	if(!Db)
		return DB_NO_FOUND;
	char **DbResult;
	int nRow, nColumn;
	char * Errmsg = NULL;
	int i, j, index, Result;
	char SqlSentence[256];
	memset(SqlSentence, 0, sizeof(SqlSentence));
	snprintf(SqlSentence, sizeof(SqlSentence), "select * from %s where Name='%s';", Table, Name); 

	Result = sqlite3_get_table( Db, SqlSentence, &DbResult, &nRow, &nColumn, &Errmsg);
	if(Result == SQLITE_OK)
	{
		if(nRow > 0 && nColumn > 0)
		{
			index = nColumn;
			for(i = 0; i < nRow; i ++)
			{
				for(j = 0; j < nColumn; j ++)		
				{
					//printf("[query]Name:%d:%s, Value:%d:%s\n", i,DbResult[j], j,DbResult[index]);
					if(j == 2)
						snprintf(Value, Size, "%s", DbResult[index]);
					index ++;
				}
			}
		}
		else
		{
			sqlite3_free_table(DbResult);
			return QUERY_NO_FOUND;
		}

	}
	else
		return QUERY_ERROR;	
	sqlite3_free_table(DbResult);
	return QUERY_OK;
}

int DatabaseDeleteParaTable(sqlite3 *Db, char *Table, char *Name)
{
	if(!Db)
		return DB_NO_FOUND;
	char *Errmsg = NULL;
	char SqlSentence[256];
	memset(SqlSentence, 0, sizeof(SqlSentence));
	snprintf(SqlSentence, sizeof(SqlSentence), "DELETE FROM %s WHERE Name = '%s';", Table, Name); 

	int Result = sqlite3_exec(Db, SqlSentence, 0, 0, &Errmsg);
	if(Result != SQLITE_OK)
		return DB_ERROR;
	return DB_OK;	
}


int DatabaseGeneralCreateDevTable(sqlite3* Db, char *Table)
{
    if(!Db)
	return DB_NO_FOUND;
    char *Errmsg = NULL;
    char SqlSentence[256];
    memset(SqlSentence, 0, sizeof(SqlSentence));
    snprintf(SqlSentence, sizeof(SqlSentence), "						\
	    CREATE Table %s(									\
	    ID INTEGER PRIMARY KEY,								\
	    UUID TEXT,										\
	    UserID TEXT,									\
	    Passwd TEXT,									\
	    UserName TEXT,									\
	    Type INTEGER									\
	    );", Table); 

    int Result = sqlite3_exec(Db, SqlSentence, 0, 0, &Errmsg);
    if(Result != SQLITE_OK)
	return DB_ERROR;
    return DB_OK;	
}

int DatabaseGeneralInsertDevTable(sqlite3* Db, char *Table, char *Uuid, char *UserID, char *Passwd, char *UserName, unsigned int Type)
{
    if(!Db)
	return DB_NO_FOUND;
    char *Errmsg = NULL;
    char SqlSentence[256];
    memset(SqlSentence, 0, sizeof(SqlSentence));
    snprintf(SqlSentence, sizeof(SqlSentence), "INSERT INTO \"%s\" ValueS(NULL, '%s', '%s', '%s', '%s', '%d');", Table, Uuid, UserID, Passwd, UserName, Type); 
    int Result = sqlite3_exec(Db, SqlSentence, 0, 0, &Errmsg);
    if(Result != SQLITE_OK)
	return DB_ERROR;
    return DB_OK;	
}


int DatabaseGeneralUpdateDevTable(sqlite3* Db, char *Table, char *Uuid, char *Passwd, char *UserName)
{
    if(!Db)
	return DB_NO_FOUND;
    unsigned int pType = 0;
    if(QUERY_OK != DatabaseGeneralQueryDevTable(Db, Table, Uuid, NULL, 0, NULL, 0, NULL, 0, NULL))
	return QUERY_NO_FOUND;
    char *Errmsg = NULL;
    char SqlSentence[256];
    memset(SqlSentence, 0, sizeof(SqlSentence));
    if(Passwd && UserName)
	snprintf(SqlSentence, sizeof(SqlSentence), "update %s set Passwd='%s' UserName='%s' where Uuid='%s';", Table, Passwd, UserName, Uuid); 
    else if(Passwd)
	snprintf(SqlSentence, sizeof(SqlSentence), "update %s set Passwd='%s' where Uuid='%s';", Table, Passwd, Uuid); 
    else if(UserName)
	snprintf(SqlSentence, sizeof(SqlSentence), "update %s set UserName='%s' where Uuid='%s';", Table, UserName, Uuid); 
    else
	return DB_ERROR;

    int Result = sqlite3_exec(Db, SqlSentence, 0, 0, &Errmsg);
    if(Result != SQLITE_OK)
	return DB_ERROR;
    return DB_OK;	
}

int DatabaseGeneralQueryDevTable(sqlite3* Db, char *Table, char *Uuid, char *UserId, int IdSize, char *Passwd, int WdSize, char *UserName, int NaSize, int *Type)
{
    if(!Db)
	    return DB_NO_FOUND;
    char **DbResult;
    int nRow, nColumn;
    char *Errmsg = NULL;
    int i, j, Index, Result;
    char SqlSentence[256];
    memset(SqlSentence, 0, sizeof(SqlSentence));
    snprintf(SqlSentence, sizeof(SqlSentence), "select * from %s where Uuid='%s';", Table, Uuid); 

    Result = sqlite3_get_table(Db, SqlSentence, &DbResult, &nRow, &nColumn, &Errmsg);
    if(Result == SQLITE_OK)
    {
	if(nRow > 0 && nColumn > 0)
	{
	    Index = nColumn;
	    for(i = 0; i < nRow; i ++)
	    {
	    	if(UserId)
	    	    snprintf(UserId, IdSize, "%s", DbResult[Index + 2]);
	    	if(Passwd)
	    	    snprintf(Passwd, WdSize, "%s", DbResult[Index + 3]);
	    	if(UserName)
	    	    snprintf(UserName, NaSize, "%s", DbResult[Index + 4]);
	    	if(Type)
	    		*Type = atoi(DbResult[Index + 5]);
#if 0
	    	for(j = 0; j < nColumn; j ++)		
	    	{
	    		//printf("[query]Name:%d:%s, Value:%d:%s\n", i,DbResult[j], j,DbResult[index]);
	    		Index ++;
	    	}
#endif
	    }
	}
	else
	{
	    sqlite3_free_table(DbResult);
	    return QUERY_NO_FOUND;
	}

    }
    else
	    return QUERY_ERROR;	
    sqlite3_free_table(DbResult);
    return QUERY_OK;
}

int DatabaseDeleteDevTable(sqlite3 *Db, char *Table, char *Uuid)
{
	if(!Db)
	    return DB_NO_FOUND;
	char *Errmsg = NULL;
	char SqlSentence[256];
	memset(SqlSentence, 0, sizeof(SqlSentence));
	snprintf(SqlSentence, sizeof(SqlSentence), "DELETE FROM %s WHERE Uuid = '%s';", Table, Uuid); 

	int Result = sqlite3_exec(Db, SqlSentence, 0, 0, &Errmsg);
	if(Result != SQLITE_OK)
		return DB_ERROR;
	return DB_OK;	
}
