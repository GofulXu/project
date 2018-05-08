#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unisd.h>
#include "tftp.h"
#include "gfudp.h"

typedef enum _tftp_mode{
MODE_NETASCII = "netascii",
MODE_OCTET = "octet",
MODE_MAIL = "mail"
}tftp_mode;


int tftp_connect()
{
	int tftp_socket = gf_udp_socket();
	gf_udp_bind(tftp_socket, inet_addr("192.168.1.108") , htons(9999));
	return tftp_socket;	
}


void send_rrq(int socket, unsigned long ip, unsigned short port, char *filename, tftp_mode RRQ_MODE)
{
	int len = 0;
	unsigned char *pkt = TftpSendBuf;
	unsigned char *xp;
	unsigned short *s = NULL;

	xp = pkt;
	s  = (unsigned short *)pkt;
	*s ++ = htons(TFTP_RRQ);
	pkt = (unsigned char *)s;
	strcpy((char *)pkt,tftp_filename);
	pkt += strlen(tftp_filename) + 1;
	strcpy((char *)pkt,"octet");
	pkt += 5/*strlen("octet")*/ + 1;
	strcpy((char *)pkt,"timeout");
	pkt += 7/*strlen("timeout")*/ + 1;
	sprintf((char *)pkt,"%lu",TftpTimeoutMSecs / 1000);
	pkt += strlen((char *)pkt) + 1;
	/*try for more effic .blk size*/
	pkt += sprintf((char *)pkt,"blksize%c%d%c",0,TftpBlkSizeOption,0);
	len = pkt - xp;
	gf_udp_send(socket, ip, port, buf, size);

}

int main(int argc, char *argv[])
{


	return 0;
}
