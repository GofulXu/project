#ifndef __get_time_H__
#define __get_time_H__

#include <stdio.h>  
#include <time.h>  
#include <string.h>  
extern int error_msgfd;
/********
***获取当前系统时间打印到字符串str
*******/
void get_time(char *str);
/********
***将当前系统时间打印到字符串str尾部
*******/
void cat_time(char *str);
unsigned char get_min_time(void);
#endif