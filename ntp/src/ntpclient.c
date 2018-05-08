/*
 * ntpclient.c - NTP client
 *
 * Copyright 1997, 1999, 2000  Larry Doolittle  <larry@doolittle.boa.org>
 * Last hack: 2 December, 2000
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License (Version 2,
 *  June 1991) as published by the Free Software Foundation.  At the
 *  time of writing, that license was published by the FSF with the URL
 *  http://www.gnu.org/copyleft/gpl.html, and is incorporated herein by
 *  reference.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  Possible future improvements:
 *      - Double check that the originate timestamp in the received packet
 *        corresponds to what we sent.
 *      - Verify that the return packet came from the host we think
 *        we're talking to.  Not necessarily useful since UDP packets
 *        are so easy to forge.
 *      - Complete phase locking code.
 *      - Write more documentation  :-(
 *
 *  Compile with -D_PRECISION_SIOCGSTAMP if your machine really has it.
 *  There are patches floating around to add this to Linux, but
 *  usually you only get an answer to the nearest jiffy.
 *  Hint for Linux hacker wannabes: look at the usage of get_fast_time()
 *  in net/core/dev.c, and its definition in kernel/time.c .
 *
 *  If the compile gives you any flak, check below in the section
 *  labelled "XXXX fixme - non-automatic build configuration".
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#ifdef _PRECISION_SIOCGSTAMP
#include <sys/ioctl.h>
#endif
#include "ntpclient.h"

#define ENABLE_DEBUG
#ifdef GOEFUL_TEST
static uint32_t incoming_word[325];
#define incoming ((char *) incoming_word)
#define sizeof_incoming (sizeof(incoming_word)*sizeof(uint32_t))
static uint32_t m_new_time = 0;
#else
char incoming[1500];

#endif
extern char *optarg;

/* XXXX fixme - non-automatic build configuration */
#ifdef linux
typedef u_int32_t __u32;
#include <sys/timex.h>
#else
extern int h_errno;
#define herror(hostname) \
    fprintf(stderr,"Error %d looking up hostname %s\n", h_errno,hostname)
typedef uint32_t __u32;
#endif

#define JAN_1970        0x83aa7e80      /* 2208988800 1970 - 1900 in seconds */

/* How to multiply by 4294.967296 quickly (and not quite exactly)
 * without using floating point or greater than 32-bit integers.
 * If you want to fix the last 12 microseconds of error, add in
 * (2911*(x))>>28)
 */
#define NTPFRAC(x) ( 4294*(x) + ( (1981*(x))>>11 ) )

/* The reverse of the above, needed if we want to set our microsecond
 * clock (via settimeofday) based on the incoming time in NTP format.
 * Basically exact.
 */
#define USEC(x) ( ( (x) >> 12 ) - 759 * ( ( ( (x) >> 10 ) + 32768 ) >> 16 ) )

/* Converts NTP delay and dispersion, apparently in seconds scaled
 * by 65536, to microseconds.  RFC1305 states this time is in seconds,
 * doesn't mention the scaling.
 * Should somehow be the same as 1000000 * x / 65536
 */
#define sec2u(x) ( (x) * 15.2587890625 )

struct ntptime {
    unsigned int coarse;
    unsigned int fine;
};

void send_packet(int usd);
#ifndef GOEFUL_TEST
void rfc1305print(char *data, struct ntptime *arrival);
#else
static int rfc1305print(uint32_t *data, struct ntptime *arrival, struct timeval *tv);
#endif
void udp_handle(int usd, char *data, int data_len, struct sockaddr *sa_source, int sa_len);

/* global variables (I know, bad form, but this is a short program) */
struct timeval time_of_send;
int live=0;
int set_clock=1;   /* non-zero presumably needs root privs */

#ifdef ENABLE_DEBUG
int debug=1;
#define DEBUG_OPTION "d"
#else
#define debug 0
#define DEBUG_OPTION
#endif

int contemplate_data(unsigned int absolute, double skew, double errorbar, int freq);

int get_current_freq()
{
    /* OS dependent routine to get the current value of clock frequency.
     */
#ifdef linux
    struct timex txc;
    txc.modes=0;
    if (__adjtimex(&txc) < 0) {
        perror("adjtimex"); exit(1);
    }
    return txc.freq;
#else
    return 0;
#endif
}

int set_freq(int new_freq)
{
    /* OS dependent routine to set a new value of clock frequency.
     */
#ifdef linux
    struct timex txc;
    txc.modes = ADJ_FREQUENCY;
    txc.freq = new_freq;
    if (__adjtimex(&txc) < 0) {
        perror("adjtimex"); exit(1);
    }
    return txc.freq;
#else
    return 0;
#endif
}

void send_packet(int usd)
{
    __u32 data[12];
    struct timeval now;
#define LI 0
#define VN 3
#define MODE 3
#define STRATUM 0
#define POLL 4 
#define PREC -6

    if (debug) fprintf(stderr,"Sending ...\n");
    if (sizeof(data) != 48) {
        fprintf(stderr,"size error\n");
        return;
    }
    bzero(data,sizeof(data));
    data[0] = htonl (
        ( LI << 30 ) | ( VN << 27 ) | ( MODE << 24 ) |
        ( STRATUM << 16) | ( POLL << 8 ) | ( PREC & 0xff ) );
    data[1] = htonl(1<<16);  /* Root Delay (seconds) */
    data[2] = htonl(1<<16);  /* Root Dispersion (seconds) */
    gettimeofday(&now,NULL);
    data[10] = htonl(now.tv_sec + JAN_1970); /* Transmit Timestamp coarse */
    data[11] = htonl(NTPFRAC(now.tv_usec));  /* Transmit Timestamp fine   */
    send(usd,data,48,0);
    time_of_send=now;
}


void udp_handle(int usd, char *data, int data_len, struct sockaddr *sa_source, int sa_len)
{
    struct timeval udp_arrival;
    struct ntptime udp_arrival_ntp;

#ifdef _PRECISION_SIOCGSTAMP
    if ( ioctl(usd, SIOCGSTAMP, &udp_arrival) < 0 ) {
        perror("ioctl-SIOCGSTAMP");
        gettimeofday(&udp_arrival,NULL);
    }
#else
    gettimeofday(&udp_arrival,NULL);
#endif
    udp_arrival_ntp.coarse = udp_arrival.tv_sec + JAN_1970;
    udp_arrival_ntp.fine   = NTPFRAC(udp_arrival.tv_usec);

    if (debug) {
        struct sockaddr_in *sa_in=(struct sockaddr_in *)sa_source;
        printf("packet of length %d received\n",data_len);
        if (sa_source->sa_family==AF_INET) {
            printf("Source: INET Port %d host %s\n",
                ntohs(sa_in->sin_port),inet_ntoa(sa_in->sin_addr));
        } else {
            printf("Source: Address family %d\n",sa_source->sa_family);
        }
    }
#ifndef GOEFUL_TEST
    rfc1305print(data,&udp_arrival_ntp);
#endif
}

double ntpdiff( struct ntptime *start, struct ntptime *stop)
{
    int a;
    unsigned int b;
    a = stop->coarse - start->coarse;
    if (stop->fine >= start->fine) {
        b = stop->fine - start->fine;
    } else {
        b = start->fine - stop->fine;
        b = ~b;
        a -= 1;
    }

    return a*1.e6 + b * (1.e6/4294967296.0);
}
void unpack_ntp_data(char *data)   
{   
 long long transmit_time;   
 struct tm *localtm;  

 memcpy(&transmit_time, data + 32, 4);   
 transmit_time = ntohl(transmit_time) - JAN_1970;   
 printf("transmit time : %llx\n", transmit_time);   
 //time(&time_tmp);   
 localtm= gmtime(&transmit_time);   
 printf("Year : %d month : %d dat : %d h : %d m : %d s:%d\n",   
  localtm->tm_year + 1900,   
  localtm->tm_mon + 1,   
  localtm->tm_mday,   
  localtm->tm_hour + 8,   
  localtm->tm_min,   
  localtm->tm_sec);   
}  


/* Does more than print, so this name is bogus.
 * It also makes time adjustments, both sudden (-s)
 * and phase-locking (-l).  */
#ifndef GOEFUL_TEST
void rfc1305print(char *data, struct ntptime *arrival)
{
    unpack_ntp_data(data);
/* straight out of RFC-1305 Appendix A */
    int li, vn, mode, stratum, poll, prec;
    int delay, disp, refid;
    struct ntptime reftime, orgtime, rectime, xmttime;
    double etime,stime,skew1,skew2;
    int freq;

#define Data(i) ntohl(((unsigned int *)data)[i])
    li      = Data(0) >> 30 & 0x03;
    vn      = Data(0) >> 27 & 0x07;
    mode    = Data(0) >> 24 & 0x07;
    stratum = Data(0) >> 16 & 0xff;
    poll    = Data(0) >>  8 & 0xff;
    prec    = Data(0)       & 0xff;
    if (prec & 0x80) prec|=0xffffff00;
    delay   = Data(1);
    disp    = Data(2);
    refid   = Data(3);
    reftime.coarse = Data(4);
    reftime.fine   = Data(5);
    orgtime.coarse = Data(6);
    orgtime.fine   = Data(7);
    rectime.coarse = Data(8);
    rectime.fine   = Data(9);
    xmttime.coarse = Data(10);
    xmttime.fine   = Data(11);
#undef Data

    if (set_clock) {   /* you'd better be root, or ntpclient will crash! */
        struct timeval tv_set;
        /* it would be even better to subtract half the slop */
        tv_set.tv_sec  = xmttime.coarse - JAN_1970;
        /* divide xmttime.fine by 4294.967296 */
        tv_set.tv_usec = USEC(xmttime.fine);
#if 0
        if (settimeofday(&tv_set,NULL)<0) {
            perror("settimeofday");
            exit(1);
        }
#endif
        if (debug) {
            printf("set time to %lu.%.6lu\n", tv_set.tv_sec, tv_set.tv_usec);
        }
    }

    if (debug) {
    printf("LI=%d  VN=%d  Mode=%d  Stratum=%d  Poll=%d  Precision=%d\n",
        li, vn, mode, stratum, poll, prec);
    printf("Delay=%.1f  Dispersion=%.1f  Refid=%u.%u.%u.%u\n",
        sec2u(delay),sec2u(disp),
        refid>>24&0xff, refid>>16&0xff, refid>>8&0xff, refid&0xff);
    printf("Reference %u.%.10u\n", reftime.coarse, reftime.fine);
    printf("Originate %u.%.10u\n", orgtime.coarse, orgtime.fine);
    printf("Receive   %u.%.10u\n", rectime.coarse, rectime.fine);
    printf("Transmit  %u.%.10u\n", xmttime.coarse, xmttime.fine);
    printf("Our recv  %u.%.10u\n", arrival->coarse, arrival->fine);
    }
    etime=ntpdiff(&orgtime,arrival);
    stime=ntpdiff(&rectime,&xmttime);
    skew1=ntpdiff(&orgtime,&rectime);
    skew2=ntpdiff(&xmttime,arrival);
    freq=get_current_freq();
    if (debug) {
    printf("Total elapsed: %9.2f\n"
           "Server stall:  %9.2f\n"
           "Slop:          %9.2f\n",
        etime, stime, etime-stime);
    printf("Skew:          %9.2f\n"
           "Frequency:     %9d\n"
           " day   second     elapsed    stall     skew  dispersion  freq\n",
        (skew1-skew2)/2, freq);
    }
    /* Not the ideal order for printing, but we want to be sure
     * to do all the time-sensitive thinking (and time setting)
     * before we start the output, especially fflush() (which
     * could be slow).  Of course, if debug is turned on, speed
     * has gone down the drain anyway. */
    if (live) {
        int new_freq;
        new_freq = contemplate_data(arrival->coarse, (skew1-skew2)/2,
            etime+sec2u(disp), freq);
        if (!debug && new_freq != freq) set_freq(new_freq);
    }
    printf("%d %5d.%.3d  %8.1f %8.1f  %8.1f %8.1f %9d\n",
        arrival->coarse/86400+15020, arrival->coarse%86400,
        arrival->fine/4294967, etime, stime,
        (skew1-skew2)/2, sec2u(disp), freq);
    fflush(stdout);

}
#else
static int rfc1305print(uint32_t *data, struct ntptime *arrival, struct timeval *tv)
{
/* straight out of RFC-1305 Appendix A */
	int li, vn, mode, stratum, poll, prec;
	int delay, disp, refid;
	struct ntptime reftime, orgtime, rectime, xmttime;
	double el_time,st_time,skew1,skew2;
	int freq;

#define Data(i) ntohl(((uint32_t *)data)[i])
	li      = Data(0) >> 30 & 0x03;
	vn      = Data(0) >> 27 & 0x07;
	mode    = Data(0) >> 24 & 0x07;
	stratum = Data(0) >> 16 & 0xff;
	poll    = Data(0) >>  8 & 0xff;
	prec    = Data(0)       & 0xff;
	if (prec & 0x80) prec|=0xffffff00;
	delay   = Data(1);
	disp    = Data(2);
	refid   = Data(3);
	reftime.coarse = Data(4);
	reftime.fine   = Data(5);
	orgtime.coarse = Data(6);
	orgtime.fine   = Data(7);
	rectime.coarse = Data(8);
	rectime.fine   = Data(9);
	xmttime.coarse = Data(10);
	xmttime.fine   = Data(11);
#undef Data

	if( tv )	/* 输出时间服务器的时间 */
	{
		tv->tv_sec  = xmttime.coarse - JAN_1970;
		tv->tv_usec = USEC(xmttime.fine);
	}
	/*else*/{   /* you'd better be root, or ntpclient will crash! */
		struct timeval tv_set;
		/* it would be even better to subtract half the slop */
		tv_set.tv_sec  = xmttime.coarse - JAN_1970;
		/* divide xmttime.fine by 4294.967296 */
		tv_set.tv_usec = USEC(xmttime.fine);

		m_new_time = xmttime.coarse - JAN_1970;

#if 0
		if (settimeofday(&tv_set, NULL)<0) {
			perror("settimeofday");
			return -1;;
		}
#endif

		if (debug) {
			MODULES_LOG_DEBUG("set time to %lu.%.6lu\n", tv_set.tv_sec, tv_set.tv_usec);
		}
	}

	if (debug) {
	MODULES_LOG_DEBUG("LI=%d  VN=%d  Mode=%d  Stratum=%d  Poll=%d  Precision=%d\n",
		li, vn, mode, stratum, poll, prec);
	MODULES_LOG_DEBUG("Delay=%.1f  Dispersion=%.1f  Refid=%u.%u.%u.%u\n",
		sec2u(delay),sec2u(disp),
		refid>>24&0xff, refid>>16&0xff, refid>>8&0xff, refid&0xff);
	MODULES_LOG_DEBUG("Reference %u.%.10u\n", reftime.coarse, reftime.fine);
	MODULES_LOG_DEBUG("Originate %u.%.10u\n", orgtime.coarse, orgtime.fine);
	MODULES_LOG_DEBUG("Receive   %u.%.10u\n", rectime.coarse, rectime.fine);
	MODULES_LOG_DEBUG("Transmit  %u.%.10u\n", xmttime.coarse, xmttime.fine);
	MODULES_LOG_DEBUG("Our recv  %u.%.10u\n", arrival->coarse, arrival->fine);
	}
	el_time=ntpdiff(&orgtime,arrival);   /* elapsed */
	st_time=ntpdiff(&rectime,&xmttime);  /* stall */
	skew1=ntpdiff(&orgtime,&rectime);
	skew2=ntpdiff(&xmttime,arrival);
	freq=get_current_freq();
	if (debug) {
	MODULES_LOG_DEBUG("Total elapsed: %9.2f\n"
	       "Server stall:  %9.2f\n"
	       "Slop:          %9.2f\n",
		el_time, st_time, el_time-st_time);
	MODULES_LOG_DEBUG("Skew:          %9.2f\n"
	       "Frequency:     %9d\n"
	       " day   second     elapsed    stall     skew  dispersion  freq\n",
		(skew1-skew2)/2, freq);
	}
	/* Not the ideal order for printing, but we want to be sure
	 * to do all the time-sensitive thinking (and time setting)
	 * before we start the output, especially fflush() (which
	 * could be slow).  Of course, if debug is turned on, speed
	 * has gone down the drain anyway. */
	if (live) {
		int new_freq;
		new_freq = contemplate_data(arrival->coarse, (skew1-skew2)/2,
			el_time+sec2u(disp), freq);
		if (!debug && new_freq != freq) set_freq(new_freq);
	}
	MODULES_LOG_DEBUG("%d %.5d.%.3d  %8.1f %8.1f  %8.1f %8.1f %9d\n",
		arrival->coarse/86400, arrival->coarse%86400,
		arrival->fine/4294967, el_time, st_time,
		(skew1-skew2)/2, sec2u(disp), freq);
/* 	fflush(stdout); */
	return(el_time-st_time);
}

#endif

void stuff_net_addr(struct in_addr *p, char *hostname)
{
    struct hostent *ntpserver;
    ntpserver=gethostbyname(hostname);
	printf("hostent-h_name:%s\n",ntpserver->h_name);
    if (ntpserver == NULL) {
        herror(hostname);
        exit(1);
    }
    if (ntpserver->h_length != 4) {
        fprintf(stderr,"oops %d\n",ntpserver->h_length);
        exit(1);
    }
    memcpy(&(p->s_addr),ntpserver->h_addr_list[0],4);
}

void setup_receive(int usd, unsigned int interface, short port)
{
    struct sockaddr_in sa_rcvr;
    bzero((char *) &sa_rcvr, sizeof(sa_rcvr));
    sa_rcvr.sin_family=AF_INET;
    sa_rcvr.sin_addr.s_addr=htonl(interface);
    sa_rcvr.sin_port=htons(port);
    if(bind(usd,(struct sockaddr *) &sa_rcvr,sizeof(sa_rcvr)) == -1) {
        fprintf(stderr,"could not bind to udp port %d\n",port);
        perror("bind");
        exit(1);
    }
    listen(usd,3);
}

void setup_transmit(int usd, char *host, short port)
{
    struct sockaddr_in sa_dest;
    bzero((char *) &sa_dest, sizeof(sa_dest));
    sa_dest.sin_family=AF_INET;
    stuff_net_addr(&(sa_dest.sin_addr),host);
    sa_dest.sin_port=htons(port);
    if (connect(usd,(struct sockaddr *)&sa_dest,sizeof(sa_dest))==-1)
        {perror("connect");exit(1);}
}

#ifndef GOEFUL_TEST
void primary_loop(int usd, int num_probes, int interval)
{
    fd_set fds;
    struct sockaddr sa_xmit;
    int i, pack_len, sa_xmit_len, probes_sent;
    struct timeval to;

    if (debug) printf("Listening...\n");

    probes_sent=0;
    sa_xmit_len=sizeof(sa_xmit);
    to.tv_sec=0;
    to.tv_usec=0;
    for (;;) {
        FD_ZERO(&fds);
        FD_SET(usd,&fds);
        i=select(usd+1,&fds,NULL,NULL,&to);  /* Wait on read or error */
        if ((i!=1)||(!FD_ISSET(usd,&fds))) {
            if (i==EINTR) continue;
            if (i<0) perror("select");
            if (to.tv_sec == 0) {
                if (probes_sent >= num_probes &&
                    num_probes != 0) break;
                send_packet(usd);
                ++probes_sent;
                to.tv_sec=interval;
                to.tv_usec=0;
            }   
            continue;
        }
        pack_len=recvfrom(usd,incoming,sizeof(incoming),0,
                          &sa_xmit,&sa_xmit_len);
        if (pack_len<0) {
            perror("recvfrom");
        } else if (pack_len>0 && pack_len<sizeof(incoming)){
            udp_handle(usd,incoming,pack_len,&sa_xmit,sa_xmit_len);
        } else {
            printf("Ooops.  pack_len=%d\n",pack_len);
            fflush(stdout);
        }
        if (probes_sent >= num_probes && num_probes != 0) break;
    }
}

#else

static void get_packet_timestamp(int usd, struct ntptime *udp_arrival_ntp)
{
	struct timeval udp_arrival;
#ifdef _PRECISION_SIOCGSTAMP
	if ( ioctl(usd, SIOCGSTAMP, &udp_arrival) < 0 ) {
		perror("ioctl-SIOCGSTAMP");
		gettimeofday(&udp_arrival,NULL);
	}
#else
	gettimeofday(&udp_arrival,NULL);
#endif
	udp_arrival_ntp->coarse = udp_arrival.tv_sec + JAN_1970;
	udp_arrival_ntp->fine   = NTPFRAC(udp_arrival.tv_usec);
}

static void check_source(int data_len, struct sockaddr *sa_source, int sa_len)
{
	/* This is where one could check that the source is the server we expect */
	if (debug) {
		struct sockaddr_in *sa_in=(struct sockaddr_in *)sa_source;
		MODULES_LOG_DEBUG("packet of length %d received\n",data_len);
		if (sa_source->sa_family==AF_INET) {
			char dst[32];
			memset(dst, 0, sizeof(dst));
			inet_ntop(AF_INET, &sa_in->sin_addr, dst, sizeof(dst));
			MODULES_LOG_DEBUG("Source: INET Port %d host %s\n",
				ntohs(sa_in->sin_port),dst);
		} else {
			MODULES_LOG_DEBUG("Source: Address family %d\n",sa_source->sa_family);
		}
	}
}
int primary_loop(int usd, int num_probes, int interval, int goodness, struct timeval *tv)
{
	fd_set fds;
	struct sockaddr sa_xmit;
	int i, pack_len, sa_xmit_len, probes_sent, error;
	struct timeval to;
	struct ntptime udp_arrival_ntp;

	if (debug) MODULES_LOG_DEBUG("Listening...\n");

	probes_sent=0;
	sa_xmit_len=sizeof(sa_xmit);
	to.tv_sec=0;
	to.tv_usec=0;
	for (;;) 
	{
		FD_ZERO(&fds);
		FD_SET(usd,&fds);
		i=select(usd+1,&fds,NULL,NULL,&to);  /* Wait on read or error */
		if ((i!=1)||(!FD_ISSET(usd,&fds))) 
		{
			if (i==EINTR) 
			{
				continue;

			}
			if (i<0) perror("select");
			if (to.tv_sec == 0) {
				if (probes_sent >= num_probes &&
						num_probes != 0)
				{
					break;
				}

				send_packet(usd);
				++probes_sent;
				to.tv_sec=interval;
				to.tv_usec=0;
			}	

			continue;
		}
		pack_len=recvfrom(usd,incoming,sizeof_incoming,0,
				&sa_xmit,(socklen_t*)&sa_xmit_len);
		error = goodness+1;
		if (pack_len<0) 
		{
			perror("recvfrom");

		} 
		else if (pack_len>0 && (unsigned)pack_len<sizeof_incoming)
		{
			get_packet_timestamp(usd, &udp_arrival_ntp);
			check_source(pack_len, &sa_xmit, sa_xmit_len);
			error = rfc1305print(incoming_word, &udp_arrival_ntp, tv);
			/* udp_handle(usd,incoming,pack_len,&sa_xmit,sa_xmit_len); */
			return 0;

		} 
		else 
		{
			printf("Ooops.  pack_len=%d\n",pack_len);
			fflush(stdout);
		}
		if ( error < goodness && goodness != 0) 
			break;
		if (probes_sent >= num_probes && num_probes != 0) 
		{

			break;
		}
	}

	return -1;
}

#endif

void do_replay(void)
{
    char line[100];
    int n, day, freq, absolute;
    float sec, etime, stime, disp;
    double skew, errorbar;
    int simulated_freq = 0;
    unsigned int last_fake_time = 0;
    double fake_delta_time = 0.0;

    while (fgets(line,sizeof(line),stdin)) {
        n=sscanf(line,"%d %f %f %f %lf %f %d",
            &day, &sec, &etime, &stime, &skew, &disp, &freq);
        if (n==7) {
            fputs(line,stdout);
            absolute=(day-15020)*86400+(int)sec;
            errorbar=etime+disp;
            if (debug) printf("contemplate %u %.1f %.1f %d\n",
                absolute,skew,errorbar,freq);
            if (last_fake_time==0) simulated_freq=freq;
            fake_delta_time += (absolute-last_fake_time)*((double)(freq-simulated_freq))/65536;
            if (debug) printf("fake %f %d \n", fake_delta_time, simulated_freq);
            skew += fake_delta_time;
            freq = simulated_freq;
            last_fake_time=absolute;
            simulated_freq = contemplate_data(absolute, skew, errorbar, freq);
        } else {
            fprintf(stderr,"Replay input error\n");
            exit(2);
        }
    }
}

void usage(char *argv0)
{
    fprintf(stderr,
    "Usage: %s [-c count] [-d] -h hostname [-i interval] [-l]\n"
    "\t[-p port] [-r] [-s] \n",
    argv0);
}

