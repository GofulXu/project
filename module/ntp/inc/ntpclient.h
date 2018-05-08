#ifndef __NTFCLIENT_H__
#define __NTPCLIENT_H__

 #define MODULES_LOG_DEBUG( format, ... )  printf("OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )

#define NTP_PORT (123)
void setup_receive(int usd, unsigned int interface, short port);


void setup_transmit(int usd, char *host, short port);


#ifndef GOEFUL_TEST
void primary_loop(int usd, int num_probes, int interval);
#else
int primary_loop(int usd, int num_probes, int interval, int goodness, struct timeval *tv);

#endif
#endif /*__NTPCLIENT_H__*/
