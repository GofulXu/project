//有名管道读文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int arg, char *args[])
{
    if (arg < 2)
    {
        printf("请输入一个参数！\n");
        return -1;
    }
    int fd = 0, rsize;
    char buf[1024] = { 0 };
    //打开管道文件
    fd = open(args[1], O_RDONLY);
	char filen[64];
	snprintf(filen, sizeof(filen), "%s.bk", args[1]);
	FILE* fq = fopen(filen, "w+");
    while ((rsize = read(fd, buf, sizeof(buf))) > 0)
    {
		fwrite(buf, 1, rsize, fq);
        memset(buf, 0, sizeof(buf));
    }
    //close the file stream
	fclose(fq);
    close(fd);
    return 0;
}
