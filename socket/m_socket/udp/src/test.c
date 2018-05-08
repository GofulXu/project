#include "default/gfapi.h"
#include "gfudp.h"

#define MULTICAST_IP "234.5.5.5"
#define MULTICAST_PORT 5555

int main(int argc, char *argv[])
{
	char data[1024];
	int i = 0;
	fd_set readfds;
	int sockfd =  gf_udp_socket();
	if(sockfd < 0)
		goto ENTER;
//	gf_udp_bind(sockfd, inet_addr("10.10.1.108"), htons(8888));
	i = gf_udp_join(sockfd, inet_addr(MULTICAST_IP));
	if(i < 0)
		goto ENTER;

	i = 30;
	while(i--)
	{
		if(gf_udp_select(sockfd, &readfds, NULL, NULL, 2000))
		{
			memset(data, 0, sizeof(data));
			gf_udp_recv(sockfd, NULL, NULL, data, sizeof(data));
			printf("data recv: %s\n", data);
		}
		printf("select\n");
		sleep(1);
	}
ENTER:
	if(sockfd)
		gf_udp_close(sockfd);

	return 0;
}
