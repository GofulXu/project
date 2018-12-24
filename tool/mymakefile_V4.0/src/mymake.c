#include "gfapi.h"

#define INTERFACE_PATH "/home/goeful/tool/mymakefile_V4.0/bin/interface.tar"

int main(int argc, char **argv)//参数个数， 参数列表
{
	char cmd[1024];
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "tar xvf %s", INTERFACE_PATH);
	printf("\ncmd:%s\n", cmd);
	system(cmd);
	return 0;
}
