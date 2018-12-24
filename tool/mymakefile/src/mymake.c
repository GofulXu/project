#include "default/gfapi.h"

#define MAKEFILE_GCC "/home/goeful/tool/mymakefile/src/Makefile_gcc"
#define MAKEFILE_FSL "/home/goeful/tool/mymakefile/src/Makefile_fsl"
#define MAKEFILE_HISI "/home/goeful/tool/mymakefile/src/Makefile_hisi"
#define MAKEFILE_AR9331 "/home/goeful/tool/mymakefile/src/Makefile_ar9331"
#define MAKEFILE_MT7628 "/home/goeful/tool/mymakefile/src/Makefile_mt7628"
#define README_PATH "/home/goeful/tool/mymakefile/src/readme.goeful"
#define TEST_PATH "/home/goeful/tool/mymakefile/src/test.goeful"
#define INC_CMD "cp /home/goeful/tool/mymakefile/src/default/ ./inc/default -rf"

int copy_file(char *rfilename,char *wfilename);
//./mycp  source  dest
int main(int argc, char **argv)//参数个数， 参数列表
{
	//printf("argc=%d\n", argc);
	//printf("%s--%s--%s\n", argv[0], argv[1], argv[2]);
	char Make_rd[128] = {0};
	//判断参数 
	if(argc == 1)
	{
		strcpy(Make_rd,MAKEFILE_GCC);
	}else if(strcmp(argv[1], "fsl") == 0)
	{
		strcpy(Make_rd,MAKEFILE_FSL);
	}else if(strcmp(argv[1],"gcc") == 0)
	{
		strcpy(Make_rd, MAKEFILE_GCC);
	}else if(strcmp(argv[1],"hisi") == 0)
	{
		strcpy(Make_rd, MAKEFILE_HISI);
	}else if(strcmp(argv[1],"ar9331") == 0)
	{
		strcpy(Make_rd, MAKEFILE_AR9331);
	}else if(strcmp(argv[1],"mt7628") == 0)
	{
		strcpy(Make_rd, MAKEFILE_MT7628);
	}else if(strstr(argv[1],"help") != NULL)
	{
		printf("\n\tgcc to create project for linux");
		printf("\n\tfsl to create project for arm linux fsl");
		printf("\n\thisi to create project for arm linux hisi");
		printf("\n\tno parameter than create project for linux");
	}else
	{
		perror("\ncreate project error!");
		return -1;
	}

	if(opendir("./bin") == NULL)
	{
		mkdir("./bin", 0775);
	}
	if(opendir("./libs") == NULL)
	{
		mkdir("./libs", 0775);
	}
	if(opendir("./inc") == NULL)
	{
		mkdir("./inc", 0775);
	}
	if(opendir("./src") == NULL)
	{
		mkdir("./src", 0775);
	}
	if(opendir("./doc") == NULL)
	{
		mkdir("./doc", 0775);
	}
	if(!access("./Makefile", F_OK))
	{
		remove("./Makefile");
	}
	if(access("./doc/readme.txt", F_OK))
	{
		if(copy_file(README_PATH, "./doc/readme.txt") < 0)return -1;
	}

	if(copy_file(Make_rd, "./Makefile") < 0)return -1;
	if(copy_file(TEST_PATH, "./src/test.c") < 0)return -1;
	system(INC_CMD);
	return 0;
}

int copy_file(char *rfilename,char *wfilename)
{

	//打开目标文件
	int srcfd = open(rfilename,O_RDONLY);
	int destfd = open(wfilename, O_WRONLY|O_CREAT,00777);
	if(destfd < 0 || srcfd < 0)
	{
		perror("dest create fial");
		printf("srcfd:%d, destfd:%d, %s %s", srcfd, destfd, rfilename, wfilename);
		close(srcfd);
		return -1;
	}

	//读数据
	//获取页大小
	int size = getpagesize();
	char buf[size*10];
	memset(buf, 0, size);
	int ret = 0;
	while((ret = read(srcfd, buf, sizeof(buf))) != 0)
	{
		ret = write(destfd, buf, ret);
	}

	close(srcfd);
	close(destfd);
	return 0;
}

	
