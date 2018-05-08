#include <stdio.h>
#include "swthrd.h"
#include "swtype.h"
#include <stdlib.h>
#include <unistd.h>
#include "swlog.h"
#define SW_LOG_TARGET_NULL		0x0
#define SW_LOG_TARGET_CONSOLE	0x01
#define SW_LOG_TARGET_FILE		0x02
#define SW_LOG_TARGET_SYSLOGD	0x04
#define SW_LOG_TARGET_SNMPTRAP	0x08
#define SW_LOG_TARGET_SMTP		0x10
#define SW_LOG_TARGET_UDP		0x20

bool test_goeful(uint32_t a, uint32_t b)
{
	/*do some thing*/
	//printf("test_goeful: a = %d, b = %d\n",a ,b);
	switch(a%6)
	{
		case 0:sw_log(LOG_LEVEL_ALL, "goeful", __FUNCTION__, __LINE__, "\n\tGoeful ---------------->>> %d\n", a);break;
		case 1:sw_log(LOG_LEVEL_INFO, "Agoeful", __FUNCTION__, __LINE__, "\n\tGoeful ---------------->>> %d\n", a);break;
		case 2:sw_log(LOG_LEVEL_DEBUG, "Bgoeful", __FUNCTION__, __LINE__, "\n\tGoeful ---------------->>> %d\n", a);break;
		case 3:sw_log(LOG_LEVEL_WARN, "Cgoeful", __FUNCTION__, __LINE__, "\n\tGoeful ---------------->>> %d\n", a);break;
		case 4:sw_log(LOG_LEVEL_ERROR, "Dgoeful", __FUNCTION__, __LINE__, "\n\tGoeful ---------------->>> %d\n", a);break;
		case 5:sw_log(LOG_LEVEL_FATAL, "Egoeful", __FUNCTION__, __LINE__, "\n\tGoeful ---------------->>> %d\n", a);break;
	
	}
	sw_thrd_delay(100);
	return true;
}

int main(void)
{
	int i = 0;
	if(!sw_log_init(LOG_LEVEL_DEBUG, "console", "goeful"))printf("log init success\n");
	sw_log_add_mod("Agoeful");
	sw_log_add_mod("Bgoeful");
	sw_log_add_mod("Cgoeful");
	sw_log_add_mod("Dgoeful");
	sw_log_add_mod("Egoeful");
	printf("target:%s\n", sw_log_get_targets());
	printf("mods:%s\n", sw_log_get_mods());
	printf("level:%d\n", sw_log_get_level());
	HANDLE ret[10] = {0};
		//创建线程并暂停
	for(i = 3; i > 0; i--)
	{
		ret[i] = sw_thrd_open("goeful", DEFAULT_PRIORIOTY, SW_SCHED_NORMAL, DEFAULT_STACK_SIZE,test_goeful, i,2);
		if(NULL != ret[i]){
			sw_thrd_resume(ret[i]);	//开始运行线程
		}
	}
	
	sw_thrd_delay(2000);
	sw_thrd_print();	//打印线程信息

	for(i = 3; i > 0; i--)
	{
		sw_thrd_close(ret[i], 0);
	
	}	
	
	sw_log_exit();
	return 0;
}
