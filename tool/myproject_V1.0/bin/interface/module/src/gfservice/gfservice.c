#include <sys/un.h>
#include "gfapi.h"
#include "gfthrd.h"
#include "gfmutex.h"
#include "gfparamerter.h"
#include "gfservice.h"
#include "gfadjsystime.h"
#include "gfhashfunc.h"

#define READSOCK_PATH "/home/.mediatest"
#define WRITESOCK_PATH "/home/.devmanager"
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path) + strlen ((ptr)->sun_path))


static HANDLE m_thrd=NULL;
static int m_read_fd=0;
static int m_write_fd=0;
static	 struct  sockaddr_un m_servaddr;
static char recv_buf[1024*2],send_buf[1024*2];

static bool m_exit = false;

static int init_recv_socket( char * path )
{
    int sockfd,len;
    struct sockaddr_un addr;
    sockfd=socket(AF_UNIX,SOCK_DGRAM,0);

    if(sockfd<0)
    {
		printf( "create unix socket failed path=%s,erron=%d erro message=%s\n",path,errno,strerror(errno));
        return -1;
    }
    bzero(&addr,sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    unlink(path);
    len = strlen(addr.sun_path) + sizeof(addr.sun_family);

    if(bind(sockfd,(struct sockaddr *)&addr,len)<0)
    {
     	printf("bind unix socket path=%s bind failed error no=%d,error message=%s \n",path,errno,strerror(errno));
        close(sockfd);
      return -1;
    }

    return sockfd;
}

static bool service_event_proc (uint64_t wpara,uint64_t lpara )
{
	int count = 0;
	char func_name[64], func_value[128];
	if( m_read_fd <= 0 )
			m_read_fd=init_recv_socket( READSOCK_PATH );
    if( m_read_fd > 0 )
    {
        while(! m_exit )
        {       
            memset(recv_buf, 0,sizeof(recv_buf));

            if(recvfrom(m_read_fd, recv_buf, sizeof(recv_buf), 0, NULL, NULL) < 0)			
			{
				count++;
                continue;
			}
			memset(func_name, 0, sizeof(func_name));
			memset(func_value, 0, sizeof(func_value));
            memset(send_buf, 0, sizeof(send_buf));

            printf("service recv_buf --------------------->*%s*\n", recv_buf);
			if(2 != sscanf(recv_buf, "%[^&]&%[^&]", func_name, func_value))
			{
				snprintf(send_buf, sizeof(send_buf), "operate_parsefail");
			}else
			{
				if(!strncmp(func_name, "gf_system", strlen("gf_system")))
				{
					system(func_value);
				}else
				{
					char re_value[1024];
					memset(re_value, 0, sizeof(re_value));
					if(!gf_douthash_function(func_name, func_value, re_value, sizeof(re_value)))
						snprintf(send_buf, sizeof(send_buf), "operate_suc&%s", re_value);
					else
						snprintf(send_buf, sizeof(send_buf), "operate_nofound");
				}
			}
			if(send_buf[0] == '\0')
			{
				snprintf(send_buf, sizeof(send_buf), "operate_ok");
			}

            if(0 > sendto(m_write_fd, send_buf, strlen(send_buf), 0, (struct sockaddr *)&m_servaddr, SUN_LEN(&m_servaddr) ))
			{
				count++;
			}

			if(count > 10)
				break;
        }
    }
	return false;
}

int gf_service_init()
{
    m_read_fd=init_recv_socket( READSOCK_PATH );
	m_write_fd = socket(AF_UNIX, SOCK_DGRAM,0);
	m_servaddr.sun_family = AF_LOCAL;
	strcpy(m_servaddr.sun_path, WRITESOCK_PATH);

    m_thrd=gf_thrd_open("devmontior_event_proc",100,0,100, service_event_proc, NULL, 0,0);
    if( m_thrd ==NULL )
		printf("don't create a thrd for handle network event\n");
	else
		gf_thrd_resume( m_thrd );

    return 0;
}

bool gf_service_check()
{
	return gf_thrd_is_running(m_thrd);
}

int gf_service_exit()
{
    m_exit = true;
    close( m_write_fd );
	close( m_read_fd );
	m_write_fd=-1;
	m_read_fd=-1;
    if(m_thrd)
    {
        gf_thrd_close(m_thrd, 100);
        m_thrd = NULL;
    }
    return 0;
}


#if 0
//sample
int main(int argc, char *argv[])
{
	gf_service_init();

	while(1)
	{
		if(!gf_service_check())
		{
			gf_service_exit();
			gf_thrd_delay(5000);
			gf_service_init();
		}
		gf_thrd_delay(5000);
	}
	gf_service_exit();

	return 0;
}
#endif
