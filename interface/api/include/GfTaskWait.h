/*************************************************************************
	> File Name: ../../include/GfTaskWait.h
	> Author: vinsion
	> Mail: vinsion@vinsion.mail 
	> Created Time: Thu 19 Sep 2019 12:50:27 PM +06
 ************************************************************************/

#ifndef __GFTASKWAIT_H__
#define	__GFTASKWAIT_H__

#define GFTASKWAIT_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "GfTaskWait", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFTASKWAIT_LOG_INFO( format, ...) 	gf_log(LOG_LEVEL_INFO, "GfTaskWait", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFTASKWAIT_LOG_WARN( format, ...) 	gf_log(LOG_LEVEL_WARN, "GfTaskWait", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFTASKWAIT_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "GfTaskWait", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFTASKWAIT_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "GfTaskWait", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

typedef int (*TaskFunProc)(const void *wParam);
//任务信息
typedef struct _TaskMsg{
    int Cmd;			//等待应答指令
    int Result;			//应答结果
    unsigned int TimeTick;	//任务开始时间
    unsigned int TimeOut;	//任务超时时间
    void *Data;			//任务反馈后传入参数
    TaskFunProc Fun;		//任务反馈后处理函数
    char IdCard[32];		//应答身份标识, 不需要判断直接清0
}TaskMsg;

int GfTaskAddResultWait(unsigned int Id, int Cmd, char *IdCard, int Result);
void GfUthashPrintfTaskWait(void);
int GfUthashAddTaskWait(TaskMsg *Msg, unsigned int *TaskId);
int GfUthashDeleteTaskWait(unsigned int Id, int Cmd, char *IdCard);
int GfUthashClearTaskWait(void);
int GfTaskWaitInit(void);
int GfTaskWaitExit(void);

#endif /*__GFTASKWAIT_H__*/
