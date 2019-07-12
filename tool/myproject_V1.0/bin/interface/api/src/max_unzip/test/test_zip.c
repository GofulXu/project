#include "gfapi.h"
#include "zip_uppack.h"

int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf("input err\tplease input app + zipfile + savepath\n");
		return 0;
	}

	struct stat mst;
	stat(argv[1], &mst);
	if(mst.st_size > 1024*1024*12)
	{
		printf("size:%ldM%ldk so big \n", mst.st_size/1024/1024, mst.st_size/(1024*1024));
		return 0;
	
	}
	char *RB = (char *)malloc((unsigned long)mst.st_size+22);

	int fq = open(argv[1], O_RDONLY);
	size_t ret = 0;
	while(ret < mst.st_size)
	{
		if(ret/1024 == mst.st_size/1024)
			ret += read(fq, RB + ret, mst.st_size - ret);
		else
			ret += read(fq, RB + ret, 1024);
		if(ret == 0)
			break;
	}
	close(fq);
#ifdef ZTASK
	Zip_head *head = zip_create_head();
	size_t num = zip_uppack_task(head, RB, ret, argv[2]);
	zip_printf_head(head);
	zip_free_head(head);
#else
	size_t num = zip_uppack(RB, ret, argv[2]);
#endif
	if(num == mst.st_size)
	{
		printf("du_zip suc num:%ld\tsize:%ld\n", num, mst.st_size);
	}
	else
	{
		printf("du_zip err num:%ld\tsize:%ld\n", num, mst.st_size);
	}

	if(NULL != RB)
		free(RB);
	RB = NULL;
	return 0;
}

