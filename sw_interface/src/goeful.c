
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "swapi_linux.h"
#include "swthrd.h"
#include "swlog.h"

static bool goefulproc(unsigned long wParam, unsigned long lParam)
{
	printf("char:%c \n", getchar());
	printf("goeful reboot test ----- >>> \n");
	sleep(8);
	//system("reboot");
	return true;
}

int main(int argc, char *argv[])
{
	HANDLE m_hthrd = sw_thrd_open("goeful", 80, 0, 1024, (PThreadHandler)goefulproc, 0, 0);
	if(m_hthrd != NULL)
	{
		sw_thrd_resume(m_hthrd);
	}
	sleep(10);
	return 0;
}
