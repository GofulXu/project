#include "gfapi.h"
#include "gfshell.h"

static int shell_test_1(int a, int b)
{
	printf("%s:%d\ta=%d\tb=%d\ta+b=%d\n", __FUNCTION__, __LINE__, a, b, a+b);
	return 0;
}

static int shell_test_2(int a, int b)
{
	printf("%s:%d\ta=%d\tb=%d\ta-b=%d\n", __FUNCTION__, __LINE__, a, b, a-b);
	return 0;
}

static void shell_exit_end(void)
{
	printf("\napp is exit\n\n");
	return ;
}

void app_shell_init(void)
{
	gf_shell_adduthash_function("shell_test1", shell_test_1);
	gf_shell_adduthash_function("shell_test2", shell_test_2);
	gf_shell_adduthash_exitfunction((exit_fun_ptr)shell_exit_end);
	return ;
}

void app_shell_run(void)
{
	gf_shell_run();
	return ;
}
