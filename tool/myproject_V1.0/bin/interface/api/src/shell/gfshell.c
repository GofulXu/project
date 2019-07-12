#include "gfapi.h"
#include "gfthrd.h"
#include "uthash/uthash.h"
#include "gfshell.h"

typedef struct _shell_function{
	char name[256];
	shell_fun_ptr fun;
	UT_hash_handle hh;
}shell_function_t;

typedef struct _shell_exitfunction{
	char name[256];
	exit_fun_ptr fun;
	UT_hash_handle hh;
}shell_exitfunction_t;

static shell_function_t *m = NULL;
static shell_exitfunction_t *mexit = NULL;
static HANDLE m_shell_thrd = NULL;

#define SHELL_EXIT_NAME	"shell_exit"
static int exit_count = 0;

static void printf_uthash_function( )
{
	printf( "\n");
	shell_function_t *current_user = NULL, *tmp = NULL;
	HASH_ITER(hh, m, current_user, tmp) 
	{
		GFSHELL_LOG_DEBUG( "%s\n", current_user->name);
	}
	printf( "\n");
	return ;
}

static void printf_uthash_exitfunction( )
{
	printf( "\n");
	shell_exitfunction_t *current_user = NULL, *tmp = NULL;
	HASH_ITER(hh, mexit, current_user, tmp) 
	{
		GFSHELL_LOG_DEBUG( "%s\n", current_user->name);
	}
	printf( "\n");
	return ;
}

static int clear_uthash_function( )
{
	shell_function_t *current_user = NULL, *tmp = NULL;
	HASH_ITER(hh, m, current_user, tmp) 
	{
		HASH_DEL(m,current_user);  /* delete; users advances to next */
		free(current_user);            /* optional- if you want to free  */
		current_user = NULL;
	}
}

static int clear_uthash_exitfunction( )
{
	shell_exitfunction_t *current_user = NULL, *tmp = NULL;
	HASH_ITER(hh, mexit, current_user, tmp) 
	{
		HASH_DEL(mexit,current_user);  /* delete; users advances to next */
		free(current_user);            /* optional- if you want to free  */
		current_user = NULL;
	}
}

static int delete_uthash_function(char *name, shell_fun_ptr mfun)
{
	shell_function_t *tmp = NULL;
	HASH_FIND_STR(m, name, tmp);
	if(!tmp)
		return -1;
	HASH_DEL(m, tmp);
	free(tmp);
	tmp = NULL;
	return 0;
}

static int delete_uthash_exitfunction(char *name, shell_fun_ptr mfun)
{
	shell_exitfunction_t *tmp = NULL;
	HASH_FIND_STR(mexit, name, tmp);
	if(!tmp)
		return -1;
	HASH_DEL(mexit, tmp);
	free(tmp);
	tmp = NULL;
	return 0;
}

static int do_uthash_function(char *cmd, int value1, int value2)
{
	shell_function_t *tmp = NULL;
	HASH_FIND_STR(m, cmd, tmp);
	if(tmp)
	{
		if(!tmp->fun(value1, value2));
		else;
	}
	else
		return -1;

	return 0;

}

static int do_uthash_exitfunction(char *cmd)
{
	shell_exitfunction_t *tmp = NULL;
	HASH_FIND_STR(mexit, cmd, tmp);
	if(tmp)
	{
		tmp->fun();
	}
	else
		return -1;

	return 0;
}

void gf_shell_exit(void)
{
	int i;
	char name[64];
	for(i = exit_count; i > 0; i--)
	{
		memset(name, 0, sizeof(name));
		snprintf(name, sizeof(name), "%s_%d", SHELL_EXIT_NAME, i);
		do_uthash_exitfunction(name);
		GFSHELL_LOG_DEBUG( "%s is exit\n", name);
	}
	clear_uthash_function();
	clear_uthash_exitfunction();
	return;
}


int gf_shell_adduthash_function(char *name, shell_fun_ptr mfun)
{
	if(strlen(name) > sizeof(m->name))
	{
		GFSHELL_LOG_DEBUG( "name :%s is too long\n", name);
		return -2;
	}

	shell_function_t *tmp = NULL;
	HASH_FIND_STR(m, name, tmp);
	if(tmp)
		return -1;
	shell_function_t *mm = (shell_function_t *)malloc(sizeof(shell_function_t));
	memset(mm, 0, sizeof(shell_function_t));
	snprintf(mm->name, sizeof(mm->name), "%s", name);
	mm->fun = mfun;
	HASH_ADD_STR(m, name, mm);
	return 0;
}

int gf_shell_adduthash_exitfunction(exit_fun_ptr mfun)
{
	exit_count++;
	shell_exitfunction_t *tmp = NULL;
	char name[64];
	memset(name, 0, sizeof(name));
	snprintf(name, sizeof(name), "%s_%d", SHELL_EXIT_NAME, exit_count);
	shell_exitfunction_t *mm = (shell_exitfunction_t *)malloc(sizeof(shell_exitfunction_t));
	memset(mm, 0, sizeof(shell_exitfunction_t));
	snprintf(mm->name, sizeof(mm->name), "%s", name);
	mm->fun = mfun;
	HASH_ADD_STR(mexit, name, mm);
	return 0;
}

void gf_shell_run()
{
	char cmd[32], buf[1024];
	int value1 = 0, value2 = 0;
	while(1)
	{
		memset(cmd, 0, sizeof(cmd));
		memset(buf, 0, sizeof(buf));
		value1 = value2 = -1;

		if(!fgets(buf, sizeof(buf), stdin))
			continue;

		if(*buf == '\0')
			continue;

		if(sscanf(buf, "%s %d %d", cmd, &value1, &value2) < 1)
			continue;
		if(strstr(cmd, "help"))
		{
			printf_uthash_function();
			continue;
		}

		if(strstr(cmd, "exit"))
		{
			gf_shell_exit();
			break;
		}

		if(0 > do_uthash_function(cmd, value1, value2))
			GFSHELL_LOG_DEBUG( "shell no found\n");

	}
	return;
}

