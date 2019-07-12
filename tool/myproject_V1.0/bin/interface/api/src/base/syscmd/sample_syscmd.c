#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "gfapi.h"


/* 将一固定的关键子作为整个系统的发送命令key,后续的key不能和此key相同 */
#define SYSCMD_SND_KEY	0x12345678
#define SYSCMD_RCV_KEY	0x87654321
/* 发送消息队列的pid */
static int m_snd_msgpid = -1;
static int m_rcv_msgpid = -1;
typedef struct 
{
	long msgtype;
	char msg[1024];
}syscmdmsg;

typedef struct
{
	syscmdmsg cmd;
	bool	buse;	//是否使用中
	int	keyval;
}syscmdinfo;

static int m_cmd_count = 0;

static syscmdinfo m_cmd_info[32];

static void kill_syscmd(int sig) 
{ 
	signal (sig, SIG_DFL); 
	/* 向线程发送信号量 */
	kill (0, sig); 
} 

static void handle_usrsignal(int sig)
{
	//signal (sig, handle_usrsignal); 
	/* 向进程组发送消息,如果没有shell命令在执行的话应该不能发kill，否则的话下次可能不能启动 */
	if ( m_cmd_count )
		kill (0, SIGINT); 
}

static void handle_signals () 
{ 
	signal (SIGHUP, kill_syscmd); 
	signal (SIGINT, handle_usrsignal); /* SIGINT Foreground Group */
	signal(SIGUSR1, handle_usrsignal);/* 部分命令启动后其前台保持不变,此时需要发送命令方发送信号给syscmd进程 */
	signal (SIGQUIT, kill_syscmd); /* Foreground Group */
	signal (SIGILL, kill_syscmd); 
	signal (SIGTRAP, kill_syscmd); 
	signal (SIGABRT, kill_syscmd); 
	signal (SIGFPE, kill_syscmd); 
	signal (SIGSEGV, kill_syscmd); 
	signal (SIGTERM, kill_syscmd); 
} 

static int syscmd_open_msg(void)
{
	if ( m_rcv_msgpid < 0 )
	{
		m_rcv_msgpid = msgget(SYSCMD_SND_KEY, IPC_CREAT | 0666);
		if ( m_rcv_msgpid < 0 )
		{
			perror("======system recv msgget error: ");
			/* 是否需要exit(); */
			return -1;
		}
	}
	if ( m_snd_msgpid < 0 )
	{
		m_snd_msgpid = msgget(SYSCMD_RCV_KEY, IPC_CREAT  | 0666 );
		if ( m_snd_msgpid < 0 )
		{
			perror("======system send msgget error: ");
			return -1;
		}
	}
	return 0;
}

static void syscmd_send_result(int keyval, int result)
{
	char buf[32];
	long *type = (long*)buf;
	*type = (long)keyval;
	buf[sizeof(long)] = (char)(result & 0xff);
	if ( msgsnd(m_snd_msgpid, buf, 1,  0) != 0 )
	{
		if ( errno == EIDRM || errno == EINVAL)
		{
			m_snd_msgpid = -1;	
			syscmd_open_msg();
			msgsnd(m_snd_msgpid, buf, 1,  0) ;
		}
		else if ( errno == EINTR )
			msgsnd(m_snd_msgpid, buf, 1,  0) ;
		perror("======system send msgsnd error: ");
	}
}
static void* syscmd_thread_handler( void* lp_param )
{
	int ret;
	syscmdinfo *cmdinfo = (syscmdinfo*)lp_param;
	if ( cmdinfo->buse == 0 )
	{
		pthread_exit(NULL);
		return NULL;
	}
	char *p = cmdinfo->cmd.msg;//strchr(cmdinfo->cmd.msg, ':');
	//p++;
	m_cmd_count++;
	ret = system( p );
	m_cmd_count--;
	syscmd_send_result(cmdinfo->keyval, ret);
	cmdinfo->buse = 0;
	pthread_exit(NULL);
	return NULL;
}
static bool systemcall(syscmdinfo *cmdinfo)
{
	char *p = cmdinfo->cmd.msg;
	cmdinfo->keyval = cmdinfo->cmd.msgtype;
	/* 创建线程处理系统命令,并发送运行结果消息给gfapp使用 */	
	if ( cmdinfo->keyval <= 0 || *p == '\0' )
	{
		syscmd_send_result(cmdinfo->keyval, -1);
		cmdinfo->buse = 0;
		return false;	
	}
	else if ( cmdinfo->keyval == 1 )
	{/* 切换控制台 */
		char *console0 = p, *console1 = NULL, *console2 = NULL;
		fprintf(stderr, "%s\n", p);
		char *t = strchr(p, ' ');
		if ( t != NULL )
		{
			*t++ = '\0';
			console1 = t;
			t = strchr(t, ' ');
			if ( t != NULL )
			{
				*t++ = '\0';
				console2 = t;
			}
		}
		if ( console0 != NULL && console1 != NULL && console2 != NULL )
		{
			/* 等待系统命令执行完 */
			kill(getpid(), SIGUSR1);
			usleep(1);
			/* 关闭本进程所有打开的文件 */
#if 0
			struct rlimit 	rl;
			rl.rlim_max = RLIM_INFINITY;
		 	getrlimit(RLIMIT_NOFILE, &rl);
		 	rl.rlim_max = (rl.rlim_max==RLIM_INFINITY) ? 1024 : rl.rlim_max;
			for (i = 0; i < rl.rlim_max; i++)   //创建守护进程完成   
			{  
				close(i);  
			}
#else
			close(0);
			close(1);
			close(2);
#endif
			if( -1 == open(console0, O_RDONLY) )
				perror("open error!");
			if( -1 == open(console1, O_RDWR) )
				perror("open error!");
			if( -1 == open(console2, O_RDWR) )
				perror("open error!");
		}
		cmdinfo->buse = 0;
		return true;
	}
	pthread_attr_t pattr;
	pthread_t tid;
	struct sched_param param;
	pthread_attr_init(&pattr);
	pthread_attr_setstacksize(&pattr, 4096);
	pthread_attr_setschedpolicy(&pattr, SCHED_RR);
	pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_DETACHED);/*此种方式有可能造成线程创建返回前就执行完成了*/
	param.__sched_priority = sched_get_priority_min(SCHED_RR);
	pthread_attr_setschedparam(&pattr, &param);
	
	if ( pthread_create( &tid, &pattr, syscmd_thread_handler, (void*)cmdinfo ) != 0 )
	{/* 直接使用系统调用,多线程调用此时将阻塞 */
		if ( cmdinfo->buse != 0 )
		{
			m_cmd_count++;
			int ret = system( p );
			m_cmd_count--;
			syscmd_send_result(cmdinfo->keyval, ret);
			cmdinfo->buse = 0;
		}
	}
	return true;
} 

void gf_syscmd_open_console(void)
{
	char console[32];
	char buf[512];
	int fd = open("/proc/cmdline", O_RDONLY);
	memset(console, 0 , sizeof(console));
	strncpy(console, "/dev/", sizeof(console)-1);
	if (fd >= 0)
	{
		int size = read(fd, buf, sizeof(buf)-1);
		char *p = NULL;
		close(fd);
		size = size > 0 ? size : 0;
		buf[size] = '\0';
		p = strstr(buf, "console=");
		if ( p != NULL )
		{
			p += strlen("console=");
			char *end = strpbrk(p, ", ");
			if (atoi(p) > 9600)
			{
				p = end;
				end = strpbrk(p, " ");
			}
			if ( end != NULL )
				p[end-p] = '\0';
			snprintf(&console[5], sizeof(console)-5, "%s", p);
		}
	}
	if (console[5] == '\0')
		strncpy(&console[5], "ttyS0", sizeof(console)-6);
	if( -1 == open(console, O_RDWR ) )
		perror("open error!");
	if( -1 == open(console, O_RDWR) )
		perror("open error!");
	if( -1 == open(console, O_RDWR) )
		perror("open error!");
}
#ifndef RLIM_INFINITY
# define RLIM_INFINITY		(~0UL)
#endif
#ifndef RLIMIT_NOFILE
# define RLIMIT_NOFILE		7	/* max number of open files */
#endif
int main( int argc, char *argv[] )
{/* 主进程不能使用信号量杀死 */
	unsigned int  i = 0;
#if 1
	//使用守候进程处理
	pid_t pid;
	struct rlimit rl;
	rl.rlim_max = RLIM_INFINITY;
	
	/* 忽视子进程Terminate或Stop的时候，SIGCHLD会发送给它的父进程 */
	signal(SIGCHLD, SIG_IGN); 
	umask(0);        //第四步  
 	getrlimit(RLIMIT_NOFILE, &rl);
 	rl.rlim_max = (rl.rlim_max==RLIM_INFINITY) ? 1024 : rl.rlim_max;
	for (i = 0; i < rl.rlim_max; i++)   //创建守护进程完成   
	{  
		close(i);  
	}
	pid = fork();
	if( pid > 0 )//是父进程，结束父进程
		exit(0);
	else if(pid< 0)//fork失败，退出
		exit(1);
	setsid();       //子进程在运行，第二步 
	pid = fork();
	if( pid > 0 )//是父进程，结束父进程
		exit(0);
	else if( pid < 0 )//fork失败，退出
		exit(1);
	chdir("/");      //第三步   
	gf_syscmd_open_console();
#else
	//usleep(10000);
#endif	
	memset(&m_cmd_info, 0, sizeof(m_cmd_info));
	if ( syscmd_open_msg() < 0 )
		return 0;
	//进入消息循环 
	handle_signals();
	while ( 1 )
	{/* 先接收 */
		for ( i = 0; i < sizeof(m_cmd_info)/sizeof(m_cmd_info[0]); i++)
			if ( m_cmd_info[i].buse == 0)
				break;
		if ( i == sizeof(m_cmd_info)/sizeof(m_cmd_info[0]) )
		{/* 等待上一消息执行完成 */
			usleep(100000);
			continue;
		}
		m_cmd_info[i].cmd.msgtype = 0;
		m_cmd_info[i].cmd.msg[0] = '\0';
		while ( m_rcv_msgpid>=0 && msgrcv(m_rcv_msgpid, &m_cmd_info[i].cmd, sizeof(m_cmd_info[i].cmd)-sizeof(long), 0, 0) == -1 )//IPC_NOERROR
		{/* 等待或者接受最先到达的消息 */
			if ( errno != EINTR )
			{
				if ( errno == EIDRM || errno == EINVAL)
				{
					m_rcv_msgpid = -1;	
					syscmd_open_msg();
				}
				m_cmd_info[i].cmd.msg[0] = '\0';
				break;
			}
			
		}
		if ( m_cmd_info[i].cmd.msg[0] == '\0' )
			continue;
		m_cmd_info[i].buse = 1;
		systemcall(&m_cmd_info[i]);
	}
}
