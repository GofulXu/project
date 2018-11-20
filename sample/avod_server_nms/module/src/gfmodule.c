#include "gfapi.h"
#include "gfthrd.h"
#include "operate.h"
#include "gfshell.h"
#include "gfparamerter.h"
#include "gftime.h"
#include "gfreportlog.h"
#include "gfcomhandle.h"
#include "gfnetworkhandle.h"
#include "gfmodule.h"


static int create_exit_function(void)
{
	gf_shell_adduthash_exitfunction((exit_fun_ptr)gf_module_exit);
	return 0;
}


int gf_module_init(void)
{
	//初始化时间相关
	gf_time_init();

	//初始化日志记录
	gf_reportlog_init();

#if 0
	//
	gf_networkhandle_init();
#endif

	//注册退出处理函数
	create_exit_function();

	return 0;
}

void gf_module_exit(void)
{
	gf_networkhandle_exit();
	gf_reportlog_exit();
	return ;
}
