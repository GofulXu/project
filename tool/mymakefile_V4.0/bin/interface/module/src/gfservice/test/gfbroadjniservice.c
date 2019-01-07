#include "gfapi.h"
#include "gfthrd.h"
#include "gfservice.h"
#include "gfhashfunc.h"

int test_setoperate(char *value, char *re_value, int re_size)
{
	printf("%s:%d--->value:%s\n", __FUNCTION__, __LINE__, value);
	if(re_value)
		snprintf(re_value, re_size, "setoperate_ok");
	return 0;
}

int test_getoperate(char *value, char *re_value, int re_size)
{
	printf("%s:%d--->value:%s\n", __FUNCTION__, __LINE__, value);
	if(re_value)
		snprintf(re_value, re_size, "RETURN_YOUR_DATA");
	return 0;
}

int gf_hashfunc_init()
{
	gf_hash_adduthash_function("test_setoperate", test_setoperate);
	gf_hash_adduthash_function("test_getoperate", test_getoperate);
	return 0;
}

int main(int argc, char *argv[])
{
	gf_hashfunc_init();
	gf_service_init();

	while(1)
	{
		if(!gf_service_check())
		{
			printf("braod jni service exit begin reinit\n");
			gf_service_exit();
			gf_thrd_delay(5000);
			gf_service_init();
		}
		gf_thrd_delay(10000);
	}
	gf_service_exit();

	gf_clearuthash_function();
	return 0;
}
