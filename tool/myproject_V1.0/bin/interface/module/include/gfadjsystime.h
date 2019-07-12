#ifndef __GFADJSYSTIME_H__
#define __GFADJSYSTIME_H__

#ifdef __cplusplus
extern "C"{
#endif

//20180101 121212
int gf_adj_strtime(char *date);

int gf_adj_time(int year, int mon, int day, int hour, int min, int sec);

#ifdef __cplusplus
}
#endif
	
#endif
