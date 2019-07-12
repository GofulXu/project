#ifndef __STDAFX_H__
#define __STDAFX_H__


#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>

#include <assert.h>



typedef unsigned long long uint64_t;

#define IS_MULTICAST_IP(ip) ( 0xE0000000<=(ip&0xFF000000) && (ip&0xFF000000)<0xF0000000 )

#endif




#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <inttypes.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <getopt.h>
#include <termios.h>
#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <arpa/inet.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <net/if.h> 
#include <net/if_arp.h> 

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/param.h> 
#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <linux/if_ether.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <linux/if_packet.h>


#define TRUE  1
#define FALSE 0

#ifndef SOCKET
#define SOCKET int
#endif
#define closesocket close
#define IS_MULTICAST_IP(ip) ( 0xE0<=(ip&0xFF) && (ip&0xFF)<0xF0 )
#define FUNC_EXPORT __attribute__((visibility("default")))


#ifndef bool
#define bool uint8_t
#endif	

#ifndef true
#define true 1
#endif	

#ifndef false
#define false 0
#endif  



#ifndef SOCKET
#define SOCKET int
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#ifndef WIN32
#ifndef HANDLE
#define HANDLE void*
#endif

#ifndef LPVOID
#define LPVOID void*
#endif
#endif

#ifndef HANLDE
#define HANLDE HANDLE
#endif

#ifndef SYSHANDLE
#define SYSHANDLE HANDLE
#endif


#ifndef BOOL
#define BOOL int
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


#ifndef SW_OK
#define SW_OK 0
#endif

#ifdef SW_ERROR
#undef SW_ERROR
#endif
#define SW_ERROR -1

#ifndef NO_WAIT
#define NO_WAIT       0
#endif

#ifndef INFINITE
#define INFINITE -1
#endif

#ifndef WAIT_FOREVER
#define WAIT_FOREVER INFINITE
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif

#define MAXINTERFACES   16 

#define memalign(num, size) malloc(size)

#ifndef ERROR
#define ERROR (-1)
#endif



