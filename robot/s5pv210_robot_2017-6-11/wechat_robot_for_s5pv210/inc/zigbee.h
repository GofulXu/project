#ifndef __ZIGBEE_H__
#define __ZIGBEE_H__

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <pthread.h>

#define ZIGBEE_DEV	"/dev/s3c2410_serial1"

#define TEMP_MAX	40
#define HUMI_MAX	60
#define MQ9_MAX		'0'
#define MQ135_MAX	'0'
#define POWER_MIN 	400

//Zig:0:23:60:1:1:1111.2222.3333.4444.5555:0000:#		//标识头：Zig	结束符：:#
//"Alarm.0  Temp.23.C  Humi.60%  MQ9.1  MQ135.1  ADC.1111.2222.3333.4444.5555  Power.1000"
typedef struct Zigbee_Msg
{
	char Alarm;
	char Temp[4];
	char Humi[4];
	char MQ9;
	char MQ135;
	char ADC[64];
	char Power[8];
	char Wor[256];
	char time[32];
	char text[256];
}Zigbee_Msg;

extern int zigbee_fd;
extern Zigbee_Msg msg;
void *zigbee_read_pthread(void *fd_arg);
int start_zigbee(void);



#endif
