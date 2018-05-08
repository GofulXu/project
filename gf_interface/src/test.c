#include <gfapi_linux.h>
#include "gfthrd.h"
#include "gftype.h"
#include "gflog.h"
#define GF_LOG_TARGET_NULL		0x0
#define GF_LOG_TARGET_CONSOLE	0x01
#define GF_LOG_TARGET_FILE		0x02
#define GF_LOG_TARGET_SYSLOGD	0x04
#define GF_LOG_TARGET_SNMPTRAP	0x08
#define GF_LOG_TARGET_SMTP		0x10
#define GF_LOG_TARGET_UDP		0x20

bool test_goeful(unsigned long c, unsigned long b)
{
	/*do some thing*/
	//printf("test_goeful: a = %d, b = %d\n",a ,b);
	static int a =0;
	switch(a%6)
	{
		case 0:gf_log(LOG_LEVEL_ALL, "goeful", __FUNCTION__, __LINE__, "\n\tGoeful ---------------->>> %d\n", a);break;
		case 1:gf_log(LOG_LEVEL_INFO, "Agoeful", __FUNCTION__, __LINE__, "\n\tGoeful ---------------->>> %d\n", a);break;
		case 2:gf_log(LOG_LEVEL_DEBUG, "Bgoeful", __FUNCTION__, __LINE__, "\n\tGoeful ---------------->>> %d\n", a);break;
		case 3:gf_log(LOG_LEVEL_WARN, "Cgoeful", __FUNCTION__, __LINE__, "\n\tGoeful ---------------->>> %d\n", a);break;
		case 4:gf_log(LOG_LEVEL_ERROR, "Dgoeful", __FUNCTION__, __LINE__, "\n\tGoeful ---------------->>> %d\n", a);break;
		case 5:gf_log(LOG_LEVEL_FATAL, "Egoeful", __FUNCTION__, __LINE__, "\n\tGoeful ---------------->>> %d\n", a);break;
	
	}
	a++;
	gf_thrd_delay(1000);
	return true;
}

int main(void)
{
	if(!gf_log_init(LOG_LEVEL_DEBUG, "console", "goeful"))printf("log init success\n");
	gf_log_add_mod("Agoeful");
	gf_log_add_mod("Bgoeful");
	gf_log_add_mod("Cgoeful");
	gf_log_add_mod("Dgoeful");
	gf_log_add_mod("Egoeful");
	//gf_log_add_target("ftp://goeful:goeful@192.168.1.108/log.log");
	printf("target:%s\n", gf_log_get_targets());
	printf("mods:%s\n", gf_log_get_mods());
	printf("level:%d\n", gf_log_get_level());
	HANDLE gg;
	//创建线程并暂停
	gg = gf_thrd_open("goeful", DEFAULT_PRIORIOTY, GF_SCHED_NORMAL, DEFAULT_STACK_SIZE,test_goeful, 2,2);
	if(NULL != gg){
		printf("resume\n");
		gf_thrd_resume(gg);	//开始运行线程
	}
	
	gf_thrd_delay(500);
	gf_thrd_print();	//打印线程信息
	getchar();
	gf_thrd_close(gg, 2000);
	
	gf_log_exit();
	return 0;
}
