#include "uthash.h"
#include <stdlib.h>   /* malloc */
#include <stdio.h>    /* printf */
typedef int (*fun_ptr)(int,int);

typedef struct example_user_t {
    int id;
    fun_ptr m_fun;
	char name[10];
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

int main(int argc,char *argv[])
{
    int i = 0;
    example_user_t *user, *tmp, *users=NULL;
	char *names[] = {"goeful","ben"};
    /* create elements */
        user = (example_user_t*)malloc(sizeof(example_user_t));
        if (user == NULL) {
            exit(-1);
		}
        user->id = 2;
		strncpy(user->name,"goeful",sizeof(user->name));
        user->m_fun = m_funca;
        HASH_ADD_INT(users,id,user);
        HASH_ADD_STR(users,name,user);
    
        user = (example_user_t*)malloc(sizeof(example_user_t));
        if (user == NULL) {
            exit(-1);
		}
        user->id = 5;
		strncpy(user->name,"ben",sizeof(user->name));
        user->m_fun = m_funcb;
        HASH_ADD_INT(users,id,user);
        HASH_ADD_STR(users,name,user);

		HASH_FIND_STR(users,"ben",user);
		user->m_fun(1,2);
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
