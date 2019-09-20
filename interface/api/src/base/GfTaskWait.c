/*************************************************************************
	> File Name: GfTask.c
	> Author: vinsion
	> Mail: vinsion@vinsion.mail 
	> Created Time: Thu 19 Sep 2019 08:43:57 AM +06
 ************************************************************************/

#include "gfapi.h"
#include "gfthrd.h"
#include "gfmutex.h"
#include "uthash/uthash.h"
#include "gfshell.h"
#include "GfTaskWait.h"

//任务信息哈希表
typedef struct _UthashTaskMsg{
    int Id;
    TaskMsg Msg;
    UT_hash_handle hh;
}UthashTaskMsg;


static UthashTaskMsg *mUthashTaskMsg = NULL;
static HANDLE mTaskThrd = NULL;
static HANDLE mMutex = NULL;

//上传任务结果, Id上传或IdCard上传
int GfTaskAddResultWait(unsigned int Id, int Cmd, char *IdCard, int Result)
{
    UthashTaskMsg *Current = NULL, *Tmp = NULL;
    if(!mUthashTaskMsg)
	return GFCODE_ERRNOFOUND;
    if(Id > 0)
    {
	HASH_FIND_INT(mUthashTaskMsg, Id, Tmp);
	Tmp->Msg.Result = Result;
	return GFCODE_SUCCESS;
    }else if(IdCard)
    {
	HASH_ITER(hh, mUthashTaskMsg, Current, Tmp) 
	{
	    if(Current->Msg.Cmd == Cmd && !strncmp(Current->Msg.IdCard, IdCard, strlen(Current->Msg.IdCard)))
	    {
		Current->Msg.Result = Result;
		return GFCODE_SUCCESS;
	    }
	}
    }else
	return GFCODE_ERRPARA;
    return GFCODE_ERRNOFOUND;

}

//任务轮询线程
static bool TaskWaitThrd(uint64_t wparam, uint64_t lparam)
{
    if(!mUthashTaskMsg)
	return false;
    UthashTaskMsg *Current = NULL, *Tmp = NULL;
    static unsigned int ThrdTick = 0;
    if(gf_thrd_get_tick() - ThrdTick > 500)
    {
	HASH_ITER(hh, mUthashTaskMsg, Current, Tmp) 
	{
	    if(gf_thrd_get_tick() - Current->Msg.TimeTick > Current->Msg.TimeOut)
	    {
		Current->Msg.Result = GFCODE_ERRTIMEOUT;
		if(Current->Msg.Fun)
		    Current->Msg.Fun(&Current->Msg);
		GfUthashDeleteTaskWait(Current->Id, 0, 0);
	    }
	    if(Current->Msg.Result != GFCODE_NODE)
	    {
		if(Current->Msg.Fun)
		    Current->Msg.Fun(&Current->Msg);
		GfUthashDeleteTaskWait(Current->Id, 0, 0);
	    }
	}
	ThrdTick = gf_thrd_get_tick();
    }
    gf_thrd_delay(100);
    return true;
}

//所有任务打印
void GfUthashPrintfTaskWait(void)
{
    if(!mUthashTaskMsg)
	return ;
    printf( "\n");
    UthashTaskMsg *Current = NULL, *Tmp = NULL;
    HASH_ITER(hh, mUthashTaskMsg, Current, Tmp) 
    {
	GFTASKWAIT_LOG_DEBUG( "IdCard:%s\tCmd:%d\tTimdTick:%d\tTimeOut:%d\n", Current->Msg.IdCard, Current->Msg.Cmd, Current->Msg.TimeTick, Current->Msg.TimeOut);
    }
    printf( "\n");
    return ;
}

//新增任务
int GfUthashAddTaskWait(TaskMsg *Msg, unsigned int *TaskId)
{
    if(!Msg)
	return GFCODE_ERRPARA;
    static unsigned int Id = 0;
    for(Id; Id < 0xFFFFFFFF; Id++)
    {
	UthashTaskMsg *Tmp = NULL;
	HASH_FIND_INT(mUthashTaskMsg, Id, Tmp);
	if(!Tmp)
	    break;
    }
    if(Id >= 0xFFFFFFFF)
	Id = 0;

    UthashTaskMsg *TaskMsg = (UthashTaskMsg *)malloc(sizeof(UthashTaskMsg));
    memset(TaskMsg, 0, sizeof(UthashTaskMsg));
    memcpy(&TaskMsg->Msg, Msg, sizeof(TaskMsg));
    TaskMsg->Id = ++Id;
    gf_mutex_lock(mMutex);
    HASH_ADD_INT(mUthashTaskMsg, TaskMsg->Id, TaskMsg);
    gf_mutex_unlock(mMutex);
    if(TaskId)
	*TaskId = TaskMsg->Id;
    return GFCODE_SUCCESS;
}

//删除任务,Id删除或IdCard删除
int GfUthashDeleteTaskWait(unsigned int Id, int Cmd, char *IdCard)
{
    UthashTaskMsg *Current = NULL, *Tmp = NULL;
    if(Id > 0)
    {
	HASH_FIND_INT(mUthashTaskMsg, Id, Tmp);
	if(!Tmp)
	    return GFCODE_ERRNOFOUND;
	gf_mutex_lock(mMutex);
	HASH_DEL(mUthashTaskMsg, Tmp);
	gf_mutex_unlock(mMutex);
	free(Tmp);
	Tmp = NULL;
	return GFCODE_SUCCESS;
    }
    else if(IdCard)
    {
	HASH_ITER(hh, mUthashTaskMsg, Current, Tmp) 
	{
	    if(Current->Msg.Cmd == Cmd && !strncmp(Current->Msg.IdCard, IdCard, strlen(Current->Msg.IdCard)))
	    {
		gf_mutex_lock(mMutex);
		HASH_DEL(mUthashTaskMsg, Current);  /* delete; users advances to next */
		gf_mutex_unlock(mMutex);
		free(Current);            /* optional- if you want to free  */
		Current = NULL;
	    }
	}
    }
    return GFCODE_ERRNOFOUND;
}

//清除所有任务
int GfUthashClearTaskWait(void)
{
    UthashTaskMsg *Current = NULL, *Tmp = NULL;
    HASH_ITER(hh, mUthashTaskMsg, Current, Tmp) 
    {
	gf_mutex_lock(mMutex);
	HASH_DEL(mUthashTaskMsg, Current);  /* delete; users advances to next */
	gf_mutex_unlock(mMutex);
	free(Current);            /* optional- if you want to free  */
	Current = NULL;
    }
    return GFCODE_SUCCESS;
}


//初始化
int GfTaskWaitInit(void)
{
    if(!mMutex)
	mMutex = gf_mutex_create();
    if(!mTaskThrd)
	mTaskThrd = gf_thrd_open("TaskThrd", 50, 0, 256*1024, TaskWaitThrd, NULL, 0, 0);
    if(mTaskThrd)
	gf_thrd_resume(mTaskThrd);
    return GFCODE_SUCCESS;
}

//退出
int GfTaskWaitExit(void)
{
    if(mTaskThrd)
	gf_thrd_close(mTaskThrd, 2000);
    mTaskThrd = NULL;

    if(mUthashTaskMsg)
	GfUthashClearTaskWait();

    if(!mMutex)
	gf_mutex_destroy(mMutex);
    mMutex = NULL;
    return GFCODE_SUCCESS;
}
