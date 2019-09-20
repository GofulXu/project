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

int GfParamerterGet(const char *Name, char *Value, int Size)
{
	if(!mDb)
		return -1;
	return DatabaseGeneralQueryParaTable(mDb, Name, Value, Size);
}


int GfParamerterGetInt(const char *Name)
{
	if(!mDb)
		return -1;
	char Sub[256] = {0};
	if(DatabaseGeneralQueryParaTable(mDb, Name, Sub, sizeof(Sub)) == QUERY_OK)
	    return atoi(Sub);
	return -2;
}

int GfParamerterInsert(const char *Name, char *Value)
{
	if(!mDb)
		return -1;
	char Sub[256], Buf[256];
	memset(Sub, 0, sizeof(Sub));
	
	if(DatabaseGeneralQueryParaTable(mDb, Name, Sub, sizeof(Sub)) == QUERY_OK)
	    return -2;
	else if(DB_OK == DatabaseGeneralInsertParaTable(mDb, Name, Value))
	    return 0;
	return -3;
}

int GfParamerterDelete(const char *Name)
{
	if(!mDb)
		return -1;

	char Sub[256], Buf[256];
	memset(Sub, 0, sizeof(Sub));
	if(!DatabaseGeneralQueryParaTable(mDb, Name, Sub, sizeof(Sub)) && !DatabaseDeleteParaTable(mDb, Name))
	    return 0;
	
	return -2;
}

int GfParamerterSet(const char *Name, char *Value)
{
	if(!mDb)
		return -1;

	char Sub[256], Buf[256];
	memset(Sub, 0, sizeof(Sub));
	memset(Buf, 0, sizeof(Buf));
	snprintf(Buf, sizeof(Buf), "%s", Value);

	if(!DatabaseGeneralQueryParaTable(mDb, Name, Sub, sizeof(Sub)))
	{
		if(strncmp(Sub, Buf, strlen(Sub)) != 0)
			DatabaseGeneralUpdateParaTable(mDb, Name, Buf);
	}
	else
	{																						
		GFPARAM_LOG_DEBUG( "update %s err no found\n", Name);										
		return -2;
	}																						
	GFPARAM_LOG_DEBUG( "update:%s->%s\n", Name, Value);											
	return 0;
}


int GfParamerterSetInt(const char *Name, int Value)
{
	if(!mDb)
		return -1;

	char Sub[256];
	char Buf[256];
	memset(Buf, 0, sizeof(Buf));
	memset(Sub, 0, sizeof(Sub));
	snprintf(Buf, sizeof(Buf), "%d", Value);
	if(!DatabaseGeneralQueryParaTable(mDb, Name, Sub, sizeof(Sub)))
	{
		if(strncmp(Sub, Buf, strlen(Sub)) != 0)
			DatabaseGeneralUpdateParaTable(mDb, Name, Buf);
	}
	else
	{																						
		GFPARAM_LOG_DEBUG( "update %s err no found\n", Name);										
		return -2;
	}																						
	GFPARAM_LOG_DEBUG( "update:%s->%s\n", Name, Value);											
	return 0;
}


int GfDeviceGet(const char *Uuid, char *UserId, int IdSize, char *PassWd, int WdSize, char *UserName, int NaSize, int *Type)
{
	if(!mDb)
		return -1;
	return DatabaseGeneralQueryDevTable(mDb, Uuid, UserId, IdSize, PassWd, WdSize, UserName, NaSize, Type);
}

int GfDeviceInsert(const char *Uuid, char *UserId, char *PassWd, char *UserName, int Type)
{
	if(!mDb)
		return -1;
	char Sub[256], Buf[256];
	memset(Sub, 0, sizeof(Sub));
	
	if(QUERY_OK == DatabaseGeneralQueryDevTable(mDb, Uuid, NULL, 0, NULL, 0, NULL, 0, NULL))
	    return -2;
	else if(DB_OK == DatabaseGeneralInsertDevTable(mDb, Uuid, UserId, PassWd, UserName, Type))
	    return 0;
	return -3;
}

int GfDeviceDelete(const char *Uuid)
{
	if(!mDb)
		return -1;

	char Sub[256], Buf[256];
	memset(Sub, 0, sizeof(Sub));
	if(!DatabaseGeneralQueryDevTable(mDb, Uuid, NULL, 0, NULL, 0, NULL, 0, NULL) && !DatabaseDeleteDevTable(mDb, Uuid)) 
	    return 0;
	
	return -2;
}

int GfDeviceSet(const char *Uuid, char *PassWd, char *UserName)
{
	if(!mDb)
		return -1;

	char OldPassWd[32] = {0}, OldUserName[32] = {0};

	if(!DatabaseGeneralQueryDevTable(mDb, Uuid, NULL, 0, OldPassWd, 0, OldUserName, 0, NULL))
	{
		if((PassWd && strncmp(PassWd, OldPassWd, strlen(PassWd))) || (UserName && strncmp(UserName, OldUserName, strlen(UserName))))
		{
		    if(PassWd)
		    {
			memset(OldPassWd, 0, sizeof(OldPassWd));
			snprintf(OldPassWd, sizeof(OldPassWd), "%s", PassWd);
		    }
		    if(UserName)
		    {
			memset(OldUserName, 0, sizeof(OldUserName));
			snprintf(OldUserName, sizeof(OldUserName), "%s", UserName);
		    }
		    DatabaseGeneralUpdateDevTable(mDb, Uuid, OldPassWd, OldUserName);
		}
	}
	else
	{																						
		GFPARAM_LOG_DEBUG( "update %s|%s|%s err no found\n", Uuid, PassWd, UserName);										
		return -2;
	}																						
	GFPARAM_LOG_DEBUG( "update:%s|%s|%s\n", Uuid, PassWd, UserName);											
	return 0;
}
