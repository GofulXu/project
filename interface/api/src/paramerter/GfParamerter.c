#include "gfapi.h"
#include "sqlite3/GfOperate.h"
#include "GfParamerter.h"

static sqlite3 *mDb = NULL;

int GfParamerterInit()
{
	mDb = DatabaseOpen(PARAM_PATH);
	if(!mDb)
		return -1;
	return 0;
}

void GfParamerterExit()
{
	if(!mDb)
		DatabaseClose(mDb);
	mDb = NULL;
	return ;
}

int GfParamerterGet(char *Name, char *Value, int Size)
{
	if(!mDb)
		return -1;
	return DatabaseGeneralQueryParaTable(mDb, GENERAL_PARA_TABLE, Name, Value, Size);
}


int GfParamerterGetInt(char *Name)
{
	if(!mDb)
		return -1;
	char Sub[256] = {0};
	if(DatabaseGeneralQueryParaTable(mDb, GENERAL_PARA_TABLE, Name, Sub, sizeof(Sub)) == QUERY_OK)
	    return atoi(Sub);
	return -2;
}

int GfParamerterInsert(char *Name, char *Value)
{
	if(!mDb)
		return -1;
	char Sub[256], Buf[256];
	memset(Sub, 0, sizeof(Sub));
	
	if(DatabaseGeneralQueryParaTable(mDb, GENERAL_PARA_TABLE, Name, Sub, sizeof(Sub)) == QUERY_OK);
	else if(DB_OK == DatabaseGeneralInsertParaTable(mDb, GENERAL_PARA_TABLE, Name, Value))
		return 0;
	return -2;
}

int GfParamerterDelete(char *Name)
{
	if(!mDb)
		return -1;

	char Sub[256], Buf[256];
	memset(Sub, 0, sizeof(Sub));
	if(!DatabaseGeneralQueryParaTable(mDb, GENERAL_PARA_TABLE, Name, Sub, sizeof(Sub)) && !DatabaseDeleteParaTable(mDb, GENERAL_PARA_TABLE, Name))
	    return 0;
	
	return -2;
}

int GfParamerterSet(char *Name, char *Value)
{
	if(!mDb)
		return -1;

	char Sub[256], Buf[256];
	memset(Sub, 0, sizeof(Sub));
	memset(Buf, 0, sizeof(Buf));
	snprintf(Buf, sizeof(Buf), "%s", Value);

	if(!DatabaseGeneralQueryParaTable(mDb, GENERAL_PARA_TABLE, Name, Sub, sizeof(Sub)))
	{
		if(strncmp(Sub, Buf, strlen(Sub)) != 0)
			DatabaseGeneralUpdateParaTable(mDb, GENERAL_PARA_TABLE, Name, Buf);
	}
	else
	{																						
		GFPARAM_LOG_DEBUG( "update %s err no found\n", Name);										
		return -2;
	}																						
	GFPARAM_LOG_DEBUG( "update:%s->%s\n", Name, Value);											
	return 0;
}


int GfParamerterSetInt(char *Name, int Value)
{
	if(!mDb)
		return -1;

	char Sub[256];
	char Buf[256];
	memset(Buf, 0, sizeof(Buf));
	memset(Sub, 0, sizeof(Sub));
	snprintf(Buf, sizeof(Buf), "%d", Value);
	if(!DatabaseGeneralQueryParaTable(mDb, GENERAL_PARA_TABLE, Name, Sub, sizeof(Sub)))
	{
		if(strncmp(Sub, Buf, strlen(Sub)) != 0)
			DatabaseGeneralUpdateParaTable(mDb, GENERAL_PARA_TABLE, Name, Buf);
	}
	else
	{																						
		GFPARAM_LOG_DEBUG( "update %s err no found\n", Name);										
		return -2;
	}																						
	GFPARAM_LOG_DEBUG( "update:%s->%s\n", Name, Value);											
	return 0;

}
