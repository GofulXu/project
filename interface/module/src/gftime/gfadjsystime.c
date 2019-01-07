#include<time.h> //C语言的头文件   
#include<stdio.h> //C语言的I/O    

//20180101 121212
int gf_adj_strtime(char *date)
{
	if(!date)
		return -1;
	int adjdate = 0, adjtime = 0;
	int ret = sscanf(date, "%d %d", &adjdate, &adjtime);
	if(ret != 2)
		return -2;
	int year = adjdate/10000;
	int mon = adjdate%10000/100;
	int day = adjdate%100;
	int hour = adjtime/10000;
	int min = adjtime%10000/100;
	int sec = adjtime%100;

	printf("date:%d-%d-%d\ttime:%d-%d-%d\n", year, mon, day, hour, min, sec);
	time_t now, adj;
	struct tm *timenow;
	struct tm stm;
	struct timespec reset;
	time(&now);   
	stm.tm_year = year - 1900;
	stm.tm_mon = mon-1;
	stm.tm_mday = day;
	stm.tm_hour = hour;
	stm.tm_min = min;
	stm.tm_sec = sec;
	adj = mktime(&stm);

	printf("now:%ld---adj:%ld\n", now, adj);

	if(adj - now > 3 || now - adj > 3)
	{
		reset.tv_sec = adj;
		reset.tv_nsec = 0;
		int ret = clock_settime(CLOCK_REALTIME, &reset);
		if(ret < 0)
		{
			printf("adj local time error return:%d\n", ret);
			return -3;
		}
	
	}

	time(&now);   
	timenow = localtime(&now);   
	printf("Adj Local time is %s\n",asctime(timenow));   
	return 0;
}
  
int gf_adj_time(int year, int mon, int day, int hour, int min, int sec)
{
	printf("date:%d-%d-%d\ttime:%d-%d-%d\n", year, mon, day, hour, min, sec);
	time_t now, adj;
	struct tm *timenow;
	struct tm stm;
	struct timespec reset;
	time(&now);   
	stm.tm_year = year - 1900;
	stm.tm_mon = mon-1;
	stm.tm_mday = day;
	stm.tm_hour = hour;
	stm.tm_min = min;
	stm.tm_sec = sec;
	adj = mktime(&stm);

	printf("now:%ld---adj:%ld\n", now, adj);

	if(adj - now > 3 || now - adj > 3)
	{
		reset.tv_sec = adj;
		reset.tv_nsec = 0;
		int ret = clock_settime(CLOCK_REALTIME, &reset);
		if(ret < 0)
		{
			printf("adj local time error return:%d\n", ret);
			return -3;
		}
	
	}

	time(&now);   
	timenow = localtime(&now);   
	printf("Adj Local time is %s\n",asctime(timenow));   
	return 0;
}
  
#if 0
int main(int argc, char *argv[])   
{
	if(argc < 2)
		return 0;
	//20180101 121212
	int date = 0, time = 0;
	sscanf(argv[1], "%d %d", &date, &time);
	adj_time(date/10000, date%10000/100, date%100, time/10000, time%10000/100, time%100);
	adj_strtime(argv[1]);
	return 0;
}
#endif
