#include "gfapi.h"
#include "gfservice.h"

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		printf("input err\n");
		return 0;
	}
	printf("argv:----->%d:%s\n", argc, argv[1]);
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	int ret = gf_sendto_service(argv[1], strlen(argv[1]), buf, sizeof(buf));
	printf("gf_sendto_service return :%d---->%s\n", ret, buf);
	return 0;
}
