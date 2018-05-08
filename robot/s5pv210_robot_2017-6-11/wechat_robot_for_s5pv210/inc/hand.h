#ifndef __HAND_H__
#define __HAND_H__

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <pthread.h>
extern unsigned char Ulm_msg[32];
#define HAND_DEV	"/dev/s3c2410_serial2"
extern int hand_fd;
void *hand_read_pthread(void *fd_arg);
int start_hand(void);



#endif
