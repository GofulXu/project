//管道写文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int arg,char * argv[])
{
    if(arg<2)
    {
        printf("请输入一个参数！\n");
        return -1;
    }
    int fd=0;
	FILE *fq;
	char *recvbuf;
	struct stat mst;
	stat(argv[2], &mst);
	recvbuf = (char *)malloc(mst.st_size);
	fq = fopen(argv[2], "rb");
	int size = fread(recvbuf, 1, mst.st_size, fq);
	if(size < mst.st_size)
		printf("read err size:%d mst->size:%ld\n", size, mst.st_size);
	fclose(fq);
	fd = open(argv[1], O_WRONLY | O_NONBLOCK);
    if(fd==-1)
    {
        printf("open the file failed ! error message :%s\n",strerror(errno));
        return -1;
    }
        //写入管道文件中
    write(fd,recvbuf,mst.st_size);
	free(recvbuf);
    //close the file stream
    close(fd);
    return 0;
}
