#include "get_time.h"   

/**************************************
**获取时间
**char *str，储存时间地址
*/
void get_time(char *str)
{  
	memset(str, 0, sizeof(str));
	time_t rawtime;
	struct tm *timeinfo;	
	time (&rawtime);  
	timeinfo = localtime(&rawtime); 
	timeinfo->tm_hour += 8;
	strcpy(str, asctime(timeinfo));
	//printf("The current date/time is: %s", asctime(timeinfo));
	return ;
	
}

/**************************************
**获取min时间
**成功返回对应时间分，失败:无
*/
unsigned char get_min_time(void)
{  
	time_t rawtime;
	struct tm *timeinfo;	
	time (&rawtime);  
	timeinfo = localtime(&rawtime); 
	//timeinfo->tm_hour += 8;
	//printf("The current date/time is: %s", asctime(timeinfo));
	return (unsigned char)timeinfo->tm_min;
	
}

/**************************************
**获取时间
**char *str，拼接储存时间地址
*/

void cat_time(char *str)
{  
	time_t rawtime;
	struct tm *timeinfo;	
	time (&rawtime);  
	timeinfo = localtime(&rawtime); 
	timeinfo->tm_hour += 8;
	strcat(str,asctime(timeinfo));
	//printf("The current date/time is: %s", asctime(timeinfo));
	return ;
}

