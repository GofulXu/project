#include "gfapi.h"
#include "gfshell.h"

int shell_test_1(int a, int b)
{
	printf("%s:%d\ta=%d\tb=%d\ta+b=%d\n", __FUNCTION__, __LINE__, a, b, a+b);
	return 0;
}

int shell_test_2(int a, int b)
{
	printf("%s:%d\ta=%d\tb=%d\ta-b=%d\n", __FUNCTION__, __LINE__, a, b, a-b);
	return 0;
}

void shell_exit_end(void)
{
	printf("\napp_exit\n\n");
	return ;
}

int main(int argc, char *argv[])
{
	gf_shell_adduthash_function("shell_test1", shell_test_1);
	gf_shell_adduthash_function("shell_test2", shell_test_2);
	gf_shell_adduthash_exitfunction(shell_exit_end);
	gf_shell_run();
	printf("gfshell_run return\n");
	return 0;
}
