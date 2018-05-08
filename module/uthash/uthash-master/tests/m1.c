#include "uthash.h"
#include <stdlib.h>   /* malloc */
#include <stdio.h>    /* printf */
#include <time.h>
typedef int (*fun_ptr)(int,int);

typedef struct example_user_t {
    int id;
    fun_ptr m_fun;
//	char name[10];
    UT_hash_handle hh;
} example_user_t;

int m_funca(int a,int b)
{
	printf("\na = %d\tb = %d\ta*b = %d\n",a,b,a*b);
	return a*b;
}

int m_funcb(int a,int b)
{
	printf("\na = %d\tb = %d\ta+b = %d\n",a,b,a+b);
	return a+b;
}

int m_funcc(int a,int b)
{
	printf("\na = %d\tb = %d\ta-b = %d\n",a,b,(unsigned int)(a-b));
	return a+b;
}

int main(int argc,char *argv[])
{
    long long i = 0;
    example_user_t *user, *tmp, *users=NULL;
	char *names[] = {"goeful","ben"};
	time_t tm;
    /* create elements */
	tm = time(&tm);
	printf("add start %s\n",ctime(&tm));
	for(i = 0; i < 100; i += 2)
	{
        user = (example_user_t*)malloc(sizeof(example_user_t));
        if (user == NULL) {
            exit(-1);
		}
        user->id = i;
	//	strncpy(user->name,"goeful:1",sizeof(user->name));
        user->m_fun = m_funca;
        HASH_ADD_INT(users,id,user);
  //      HASH_ADD_STR(users,name,user);
    
        user = (example_user_t*)malloc(sizeof(example_user_t));
        if (user == NULL) {
            exit(-1);
		}
        user->id = i+1;
	//	strncpy(user->name,"ben:2",sizeof(user->name));
        user->m_fun = m_funcb;
        HASH_ADD_INT(users,id,user);
     //   HASH_ADD_STR(users,name,user);
	}
        user = (example_user_t*)malloc(sizeof(example_user_t));
        if (user == NULL) {
            exit(-1);
		}
        user->id = 1000;
	//	strncpy(user->name,"boy",sizeof(user->name));
        user->m_fun = m_funcc;
        HASH_ADD_INT(users,id,user);
    //    HASH_ADD_STR(users,name,user);

	for(i = 102; i < 202; i += 2)
	{
        user = (example_user_t*)malloc(sizeof(example_user_t));
        if (user == NULL) {
            exit(-1);
		}
        user->id = i;
	//	strncpy(user->name,"goeful:1",sizeof(user->name));
        user->m_fun = m_funca;
        HASH_ADD_INT(users,id,user);
     //   HASH_ADD_STR(users,name,user);
    
        user = (example_user_t*)malloc(sizeof(example_user_t));
        if (user == NULL) {
            exit(-1);
		}
        user->id = i+1;
	//	strncpy(user->name,"ben:2",sizeof(user->name));
        user->m_fun = m_funcb;
        HASH_ADD_INT(users,id,user);
       // HASH_ADD_STR(users,name,user);
	}
	i = 111;
	tm = time(&tm);
	printf("add end %s\n",ctime(&tm));
		HASH_FIND_INT(users,&i,tmp);
		printf("goeful--->id:%d\tname:%s",tmp->id,"goeful");
		tmp->m_fun(1,2);
#if 0
    /* delete each even ID */
		i = 2;
        HASH_FIND_INT(users,&i,tmp);
        if (tmp != NULL) {
			tmp->m_fun(1,2);
            HASH_DEL(users,tmp);
            free(tmp);
        } else {
            printf("user id %d not found\n", i);
        }
		i = 2;
        HASH_FIND_INT(users,&i,tmp);
        if (tmp != NULL) {
			tmp->m_fun(1,2);
            HASH_DEL(users,tmp);
            free(tmp);
        } else {
            printf("user id %d not found\n", i);
        }
		i = 5;
        HASH_FIND_INT(users,&i,tmp);
        if (tmp != NULL) {
			tmp->m_fun(1,2);
            HASH_DEL(users,tmp);
            free(tmp);
        } else {
            printf("user id %d not found\n", i);
        }
#endif
#if 0    /* show the hash */
    for(user=users; user != NULL; user=(example_user_t*)(user->hh.next)) {
        printf("user %d, cookie %d\n", user->id, user->cookie);
    }
#endif
    return 0;
}
