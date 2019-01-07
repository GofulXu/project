#include "gfapi.h"
#include "gfthrd.h"
#include "uthash/uthash.h"
#include "gfhashfunc.h"

typedef struct _hash_function{
	char name[256];
	hash_fun_ptr fun;
	UT_hash_handle hh;
}hash_function_t;

static hash_function_t *m = NULL;

void gf_printf_uthash_function( )
{
	printf( "\n");
	hash_function_t *current_user = NULL, *tmp = NULL;
	HASH_ITER(hh, m, current_user, tmp) 
	{
		GFHASHFUNC_LOG_DEBUG( "%s\n", current_user->name);
	}
	printf( "\n");
	return ;
}

int gf_clearuthash_function( )
{
	hash_function_t *current_user = NULL, *tmp = NULL;
	HASH_ITER(hh, m, current_user, tmp) 
	{
		HASH_DEL(m,current_user);  /* delete; users advances to next */
		free(current_user);            /* optional- if you want to free  */
		current_user = NULL;
	}
}

int gf_delete_uthash_function(char *name)
{
	hash_function_t *tmp = NULL;
	HASH_FIND_STR(m, name, tmp);
	if(!tmp)
		return -1;
	HASH_DEL(m, tmp);
	free(tmp);
	tmp = NULL;
	return 0;
}

int gf_douthash_function(char *cmd, char *value, char *re_value, int re_size)
{
	hash_function_t *tmp = NULL;
	HASH_FIND_STR(m, cmd, tmp);
	if(tmp)
	{
		if(!tmp->fun(value, re_value, re_size));
		else;
	}
	else
		return -1;

	return 0;

}


int gf_hash_adduthash_function(char *name, hash_fun_ptr mfun)
{
	if(strlen(name) > sizeof(m->name))
	{
		GFHASHFUNC_LOG_DEBUG( "name :%s is too long\n", name);
		return -2;
	}

	hash_function_t *tmp = NULL;
	HASH_FIND_STR(m, name, tmp);
	if(tmp)
		return -1;
	hash_function_t *mm = (hash_function_t *)malloc(sizeof(hash_function_t));
	memset(mm, 0, sizeof(hash_function_t));
	snprintf(mm->name, sizeof(mm->name), "%s", name);
	mm->fun = mfun;
	HASH_ADD_STR(m, name, mm);
	return 0;
}

