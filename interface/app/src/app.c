#include "sys/time.h"
#include "sys/resource.h"
#include "gfapi.h"
#include "gflog.h"
#include "gfthrd.h"
#include "gfcomhandle.h"
#include "gfnetworkhandle.h"
#include "gfshell.h"
#include "GfParamerter.h"
#include "gftime.h"
#include "gfreportlog.h"
#include "gfmodule.h"

extern void app_shell_run(void);
extern void app_shell_init(void);

static int app_set_resource()
{
	//设置生成core文件大小为无限制，需要root权限
	struct rlimit lim;
	lim.rlim_cur = lim.rlim_max = RLIM_INFINITY;
	if (setrlimit(RLIMIT_CORE, &lim)!=0)  
	{
		GF_LOG_WARN("setrlimit failed\n");
	}   

	return 0;
}

static int gf_app_log_init()
{
	//设置日志级别,IPC swapp的日志级别无法设置
	int log_level = 6;
	char value[64];

	GfParamerterSetInt( "log_type", 0);
	gf_log_init( LOG_LEVEL_ALL, "console", "all" );

	log_level = GfParamerterGetInt("log_level");
	if(log_level < 0)
		log_level = 0;

	gf_log_set_level(log_level);

	GfParamerterSet("log_targets", "console");
	memset(value,0,sizeof(value));
	if( !GfParamerterGet("log_targets", value, sizeof(value)) )
		gf_log_set_targets( value );

	return 0;
}

static int gf_app_set_env(void)
{
	putenv("SDKPATH=/usr/sdk");
	return 0;
}

static int create_exit_function(void)
{
	gf_shell_adduthash_exitfunction((exit_fun_ptr)GfParamerterExit);
	return 0;
}

int main(int argc, char *argv[])
{
	//资源调用设置
	app_set_resource();

	//参数化参数表
	GfParamerterInit();

	//初始化打印系统
	gf_app_log_init();

	//注册退出处理函数
	create_exit_function();

	//初始化shell命令
	app_shell_init();

	//初始化中间模块
	gf_module_init();

	//运行shell
	app_shell_run();

	sync();

	return 0;
}
