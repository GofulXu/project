#include <sys/un.h>
#include "gfapi.h"
#include "gfthrd.h"
#include "gfmutex.h"
#include "gfparamerter.h"
#include "gfservice.h"

#define WRITESOCK_PATH "/home/.mediatest"
#define READSOCK_PATH "/home/.devmanager"
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path) + strlen ((ptr)->sun_path))

static int m_read_fd=0;
static int m_write_fd=0;
static	 struct  sockaddr_un m_servaddr;

static int init_recv_socket( char * path )
{
    int sockfd,len;
    struct sockaddr_un addr;
    sockfd=socket(AF_UNIX,SOCK_DGRAM,0);

    if(sockfd<0)
       return -1;
    bzero(&addr,sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    unlink(path);
    len = strlen(addr.sun_path) + sizeof(addr.sun_family);

    if(bind(sockfd,(struct sockaddr *)&addr,len)<0)
    {
        close(sockfd);
        sockfd = -1;
        return -1;
    }

    return sockfd;
}

//return
//open socket err	-1
//select err		-2
//select timeout	-5
//do operate error	-3
//parse err			-4
int gf_sendto_service(char *send_data, int size, char *recv_buf, int recv_size)
{
	int err = -1;
    char recv_data[2*1024];

    m_read_fd=init_recv_socket( READSOCK_PATH );
	m_write_fd = socket(AF_UNIX, SOCK_DGRAM,0);

	m_servaddr.sun_family = AF_LOCAL;
	strcpy(m_servaddr.sun_path, WRITESOCK_PATH);

	sendto(m_write_fd, send_data, size, 0, (struct sockaddr *)&m_servaddr, SUN_LEN(&m_servaddr) );
    
    if( m_read_fd <= 0 )
        m_read_fd=init_recv_socket( READSOCK_PATH );

    int ret = 0;
	struct timeval tv;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(m_read_fd, &rfds);
    tv.tv_sec = 5; 
    tv.tv_usec = 0;

    if( m_read_fd > 0 )
    {
        memset(recv_data, 0,sizeof(recv_data));

        if((ret = select(m_read_fd + 1, &rfds, NULL, NULL, &tv)) < 0)
		{
            printf("select error\n");
			err = -2;
		}
        else if(ret > 0 && FD_ISSET(m_read_fd, &rfds))
        {
            if(recvfrom(m_read_fd, recv_data, sizeof(recv_data), 0, NULL, NULL) < 0)
                printf("ERROR\n");
			if(!strncmp(recv_data, "operate_ok", strlen("operate_ok")))
				err = 0;
			else if(!strncmp(recv_data, "operate_suc", strlen("operate_suc")) && recv_buf != NULL)
			{
				if(recv_buf)
				{
					snprintf(recv_buf, recv_size, "%s", recv_data + strlen("operate_suc") + 1 );
				}
				err = 1;
			}
			else if(!strncmp(recv_data, "operate_doerr", strlen("operate_doerr")) && recv_buf != NULL)
			{
				err = -3;
			}
			else
				err = -4;
            printf("%s\n",recv_data);
        }
        else if(ret == 0)
		{
			err = -5;
            printf("select timeout\n");
		}
    }
    else
        printf("open socket error\n");

    close( m_write_fd );
	close( m_read_fd );
	m_write_fd=-1;
	m_read_fd=-1;
    return err;
}
