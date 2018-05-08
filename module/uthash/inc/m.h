#ifndef __M_H__
#define __M_H__
#include "uthash.h"
typedef int (*fun_ptr)(int,int);

struct my_struct {
    int ikey;                    /* key */
    fun_ptr m_fun;
    char name[10];
    UT_hash_handle hh;         /* makes this structure hashable */
};

struct my_struct *find_user(struct my_struct *g_users, int ikey);
void add_user(struct my_struct *g_users, int ikey, char *value_buf, fun_ptr m_fun);
void delete_user(struct my_struct *g_users, int ikey);
void delete_all(struct my_struct *g_users);
int sum(struct my_struct *my_data);
#endif
