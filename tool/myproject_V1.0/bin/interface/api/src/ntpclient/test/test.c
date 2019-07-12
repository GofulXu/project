#include "gfapi.h"
#include "ntpclient.h"

int main(int argc, char *argv[])
{
	long tv_sec = get_time_from_ntpserver(CHINA_NTP_SERVER1);
	struct tm * mt = gmtime(&tv_sec);
	printf("\nntp sync time success\nnew time = %04d-%02d-%02d %02d:%02d:%02d\n", 1900 + mt->tm_year, mt->tm_mon + 1, mt->tm_mday, mt->tm_hour + 8, mt->tm_min, mt->tm_sec);
	return 0;
}
