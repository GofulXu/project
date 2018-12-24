#include "gfapi.h"
#include "sqlite3/operate.h"
#include "gfparamerter.h"

static sqlite3 *m_db = NULL;

int gf_paramerter_init()
{
	m_db = database_open(PARAM_PATH);
	if(!m_db)
		return -1;
	return 0;
}

void gf_paramerter_exit()
{
	if(!m_db)
		database_close(m_db);
	m_db = NULL;
	return ;
}

int gf_paramerter_get(char *name, char *value, int size)
{
	if(!m_db)
		return -1;

	char sub[256];
	memset(sub, 0, sizeof(sub));

	if(database_general_query(m_db, SYSTEM_PARA_TABLE, name, sub, sizeof(sub)) == QUERY_OK)
	{
		memset(value, 0, size);
		snprintf(value, size, "%s", sub);
	}
	else if(database_general_query(m_db, GENERAL_PARA_TABLE, name, sub, sizeof(sub)) == QUERY_OK)
	{
		memset(value, 0, size);
		snprintf(value, size, "%s", sub);
	}
	else																					
	{
		return -5555;
	}																						
	return 0;
}


int gf_paramerter_get_int(char *name)
{
	if(!m_db)
		return -1;
	char sub[256];
	memset(sub, 0, sizeof(sub));

	if(database_general_query(m_db, SYSTEM_PARA_TABLE, name, sub, sizeof(sub)) == QUERY_OK);
	else if(database_general_query(m_db, GENERAL_PARA_TABLE, name, sub, sizeof(sub)) == QUERY_OK);
	else
		return -5555;
	return atoi(sub);
}

int gf_paramerter_insert(char *name, char *value)
{
	if(!m_db)
		return -1;
	char sub[256], buf[256];
	memset(sub, 0, sizeof(sub));
	
	if(database_general_query(m_db, SYSTEM_PARA_TABLE, name, sub, sizeof(sub)) == QUERY_OK);
	else if(database_general_query(m_db, GENERAL_PARA_TABLE, name, sub, sizeof(sub)) == QUERY_OK);
	else if(DB_OK == database_general_insert_paratable(m_db, GENERAL_PARA_TABLE, name, value))
		return 0;

	return -2;
}

int gf_paramerter_insert_system(char *name, char *value)
{
	if(!m_db)
		return -1;
	char sub[256], buf[256];
	memset(sub, 0, sizeof(sub));
	
	if(database_general_query(m_db, SYSTEM_PARA_TABLE, name, sub, sizeof(sub)) == QUERY_OK);
	else if(database_general_query(m_db, GENERAL_PARA_TABLE, name, sub, sizeof(sub)) == QUERY_OK);
	else if(DB_OK == database_general_insert_paratable(m_db, SYSTEM_PARA_TABLE, name, value))
		return 0;

	return -2;
}

int gf_paramerter_delete(char *name)
{
	if(!m_db)
		return -1;

	char sub[256], buf[256];
	memset(sub, 0, sizeof(sub));
	if(database_general_query(m_db, SYSTEM_PARA_TABLE, name, sub, sizeof(sub)) == QUERY_OK)
		database_delete(m_db, SYSTEM_PARA_TABLE, name);
	else if(database_general_query(m_db, GENERAL_PARA_TABLE, name, sub, sizeof(sub)) == QUERY_OK)
		database_delete(m_db, GENERAL_PARA_TABLE, name);
	else
		return -2;
	
	return 0;
}

int gf_paramerter_set(char *name, char *value)
{
	if(!m_db)
		return -1;

	char sub[256], buf[256];
	memset(sub, 0, sizeof(sub));
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%s", value);

	if(database_general_query(m_db, SYSTEM_PARA_TABLE, name, sub, sizeof(sub)) == QUERY_OK)
	{
		if(strncmp(sub, buf, strlen(sub)) != 0)
			database_general_update(m_db, SYSTEM_PARA_TABLE, name, buf);
	}
	else if(database_general_query(m_db, GENERAL_PARA_TABLE, name, sub, sizeof(sub)) == QUERY_OK)
	{
		if(strncmp(sub, buf, strlen(sub)) != 0)
			database_general_update(m_db, GENERAL_PARA_TABLE, name, buf);
	}
	else																					
	{																						
		GFPARAM_LOG_DEBUG( "update %s err no found\n", name);										
		return -2;
	}																						
	GFPARAM_LOG_DEBUG( "update:%s->%s\n", name, value);											
	return 0;
}


int gf_paramerter_set_int(char *name, int value)
{
	if(!m_db)
		return -1;

	char sub[256];
	char buf[256];
	memset(buf, 0, sizeof(buf));
	memset(sub, 0, sizeof(sub));
	snprintf(buf, sizeof(buf), "%d", value);
	if(database_general_query(m_db, SYSTEM_PARA_TABLE, name, sub, sizeof(sub)) == QUERY_OK)
	{
		if(strncmp(sub, buf, strlen(sub)) != 0)
			database_general_update(m_db, SYSTEM_PARA_TABLE, name, buf);
	}
	else if(database_general_query(m_db, GENERAL_PARA_TABLE, name, sub, sizeof(sub)) == QUERY_OK)
	{
		if(strncmp(sub, buf, strlen(sub)) != 0)
			database_general_update(m_db, GENERAL_PARA_TABLE, name, buf);
	}
	else																					
	{																						
		GFPARAM_LOG_DEBUG( "update %s err no found\n", name);										
		return -2;
	}																						
	GFPARAM_LOG_DEBUG( "update:%s->%s\n", name, value);											
	return 0;

}
