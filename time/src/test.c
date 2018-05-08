#include <stdio.h>


    #include<sys/time.h>
    #include<unistd.h>
#include <string.h>


int main(void)
{
    struct timeval tv;
    struct timezone tz;
     time_t now;
	//datetime[strlen(datetime)-1] = '\0';
    printf("now datetime: %p\n", datetime);
	printf("now datetime: %s\n", datetime);
    gettimeofday (&tv , &tz);
    printf("tv_sec: %d\n", tv.tv_sec) ;
    printf("tv_usec: %d\n",tv.tv_usec);
    printf("tz_minuteswest: %d\n", tz.tz_minuteswest);
    printf("tz_dsttime: %d\n",tz.tz_dsttime);
	printf("\tThis application make in %s_%s,file name is %s.\n \
	Print from the function %s,the line %d.\n", __DATE__,   \
	 __TIME__, __FILE__, __FUNCTION__, __LINE__);
	return 0;
}
