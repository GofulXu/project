#include "gfapi.h" 
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/syscall.h>

#include "gflog.h"

/* 将一固定的关键子作为整个系统的发送命令key,后续的key不能和此key相同,并且在此应用中确保keyval大于0 */
#define SYSCMD_SND_KEY	0x12345678
#define SYSCMD_RCV_KEY	0x87654321

/* 目前使用msgtype作为系统独特的system调用,一般情况下不会出现相同的msgtype,如果出现相同的msgtype的话表明系统
运行不正常了,1,2,3用作特定用途
 */
static  long  m_cmd_count = 4;
/* 发送消息队列的pid */
static int m_snd_msgpid = -1;
static int m_rcv_msgpid = -1;
/* 记录有多少个shell命令启动了，暂不考虑多线程竞争造成的不准确 */
static int m_system_count = 0;
typedef struct 
{
	long msgtype;
	char msg[1024];
}syscmdmsg;

pid_t gf_syscmd_get_pid(void);
static int syscmd_open_msg(void)
{
	if ( m_snd_msgpid < 0 )
	{
		m_snd_msgpid = msgget(SYSCMD_SND_KEY, IPC_CREAT | 0666);
		if ( m_snd_msgpid < 0 )
		{
			perror("======system send msgget error: ");
			/* 是否需要exit(); */
			return -1;
		}
	}
	if ( m_rcv_msgpid < 0 )
	{
		m_rcv_msgpid = msgget(SYSCMD_RCV_KEY, IPC_CREAT  | 0666 );
		if ( m_rcv_msgpid < 0 )
		{
			perror("======system recv msgget error: ");
			return -1;
		}
	}
	return 0;
}

static int syscmd_sndcmd(const char *cmd, int keyval)
{
	syscmdmsg mycmd;
	mycmd.msgtype = keyval;
	/* 发送系统命令格式为count:cmd后加'\0' */
	memset(mycmd.msg, 0, sizeof(mycmd.msg));
	strncpy(mycmd.msg, cmd, sizeof(mycmd.msg)-1);
	//snprintf(mycmd.msg, sizeof(mycmd.msg), "%s", keyval, cmd);
	if ( msgsnd(m_snd_msgpid, &mycmd, sizeof(mycmd)-sizeof(long), 0) != 0 )
	{
		perror("======system send msgsnd error: ");
		return -1;
	}
	return 0;
}
static int syscmd_rcvcmd(int keyval)
{
	char buf[32];
	/* 对于接收的定义为msg[1],返回0,255之一,亦只接收一个字节的数据,此会截断msgtype为keyval的超过长度的消息 */
	while ( msgrcv(m_rcv_msgpid, buf, 1, keyval, MSG_NOERROR) == -1 )
	{
		if ( errno != EINTR )
		{
			if ( errno == EIDRM || errno == EINVAL)
			{
				m_rcv_msgpid = -1;	
				syscmd_open_msg();
			}
			buf[sizeof(long)] = 0xff;
			break;
		}
		else
			fprintf(stderr, "========EINTR occur when msgrcv\n");
	}
	m_system_count--;
	return buf[sizeof(long)] != 0x00 ? -1 : 0;	
}

int gfsyscmd(const char* cmd )
{
	if( cmd == NULL || *cmd == '\0' )
		return -1;
	if ( syscmd_open_msg() < 0 )
		return -1;
	//gf_log(LOG_LEVEL_DEBUG, "porting", __FUNCTION__, __LINE__, "%s,count:%d\n", cmd, m_cmd_count);
	m_cmd_count++;
	/* 一般情况下执行keyval=m_cmd_count++不会出现中断执行的情况也可使用getpid()<<16 |(unsigned short)m_cmd_count */
	if ( m_cmd_count <= 4 ) 
		m_cmd_count = 4;
	int keyval = m_cmd_count;
	/* 需要确保keyval大于0 */
	if ( syscmd_sndcmd(cmd, keyval) < 0 )
		return -1;
	m_system_count++;
	return syscmd_rcvcmd(keyval);
}

int system (__const char *__command)
{
#if 1
	return gfsyscmd( __command );
#else
	//部分system的实现代码
	pid_t pid;
	int status;
	if(__command == NULL){
		return (1);
	}
	if((pid = fork())<0){
		status = -1;
	}
	else if(pid == 0){
		execl("/bin/sh", "sh", "-c", __command, (char *)0);
		exit(127); //子进程正常执行则不会执行此语句
	}
	else{
		while(waitpid(pid, &status, 0) < 0){
			if(errno != EINTER){
				status = -1;
				break;
			}
		}
	}
	return status;

#endif
}

bool gf_syscmd_is_waiting(void)
{
	return m_system_count == 0 ? false : true;
}

pid_t gf_syscmd_init(void)
{
	pid_t id = getpid();
	char console0[128];
	char console1[128];
	char console2[128];
	char cmd[512];
	char path[64];
	int i = 0;
	snprintf(path, sizeof(path), "/proc/%d/fd/0", id);
	i = readlink(path, console0, sizeof(console0)-1);
	i = i > 0 ? i : 0;
	console0[i] = '\0';
	snprintf(path, sizeof(path), "/proc/%d/fd/1", id);
	i = readlink(path, console1, sizeof(console1)-1);
	i = i > 0 ? i : 0;
	console1[i] = '\0';
	snprintf(path, sizeof(path), "/proc/%d/fd/2", id);
	i = readlink(path, console2, sizeof(console2)-1);
	i = i > 0 ? i : 0;
	console2[i] = '\0';
	snprintf(cmd, sizeof(cmd), "%s %s %s", console0, console1, console2);
	if ( syscmd_open_msg() < 0 )
		return 0;
	syscmd_sndcmd(cmd, 1);
	return gf_syscmd_get_pid();
}

static pid_t str_2_pid(char *str)
{
	pid_t id = 0;
	while ( *str != '\0' )
	{
		if ( *str < '0' || '9' < *str )
			return (pid_t)0;
		id = id*10 + *str - '0';
		str++;
	}
	return id;
}

pid_t gf_syscmd_get_pid(void)
{
	DIR *dp;
	struct dirent dir_entry;
	struct dirent *pdir_entry;
	char path[256];
	pid_t pid1, pid2 = 0;
	if((dp = opendir("/proc")) == NULL)
		return 0;
	while( readdir_r(dp, &dir_entry, &pdir_entry)==0 && pdir_entry) {
		if ( (pid1=str_2_pid(dir_entry.d_name)) != 0 )
		{/* 是整数 */
			snprintf(path, sizeof(path), "/proc/%d/cmdline", pid1);
			int fp = open(path, O_RDONLY);
			if ( fp >= 0 )
			{
				char buf[128];
				int size = read(fp, buf, sizeof(buf)-1);
				if ( size > 0 )
				{
					buf[size] = '\0';
					char *p = strstr(buf, "gfsyscmd");
					if ( p != NULL )
						pid2 = pid1;
				}
				close(fp);
			}
			if ( pid2 != 0 )
				break;
		}
	}
	closedir(dp);
	return pid2;
}

#if 0
static pid_t m_syscmd_pid = 0;
static void delete_socket (int sig) 
{ 
	//signal (sig, SIG_DFL); 
	/* 向线程发送信号量 */
	printf("kill(%d,%d),%d\n", m_syscmd_pid, SIGUSR1, SIGINT);
	if (0 != kill(m_syscmd_pid, SIGUSR1) )
	{
		fprintf(stderr, "error kill :%d:%d\n", m_syscmd_pid, sig);
	}
	else
		fprintf(stderr, "kill :%d:%d\n", m_syscmd_pid, sig);
	//kill (getpid (), sig);
} 
 
static void handle_signals () 
{ 
	signal (SIGINT, delete_socket); 
} 
int main( int argc, char *argv[] )
{
	m_syscmd_pid  = gf_syscmd_get_pid();
	if ( m_syscmd_pid > 0 )
		handle_signals();
	printf("SIGINT:%d, syscmd:%d\n", SIGINT, m_syscmd_pid);
	int time = (int)atoi(argv[2]);
	m_cmd_count = (int)atoi(argv[1]);
	while (time-- > 0)
	{
		system(argv[3]);
	}
	return 0;
}
#endif
