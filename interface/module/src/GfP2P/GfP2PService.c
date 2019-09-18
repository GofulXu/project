/*************************************************************************
	> File Name: p2pservice.c
	> Author: vinsion
	> Mail: vinsion@vinsion.mail 
	> Created Time: Mon 16 Sep 2019 11:28:38 AM +06
 ************************************************************************/

#include "gfapi.h"
#include "gflog.h"
#include "gfthrd.h"
#include "GfUdpClient.h"
#include "GfParamerter.h"
#include "gfreportlog.h"
#include "GfCRC32.h"
#include "GfP2PService.h"
#include "uuid/uuid.h"

SUdpClient *mPClient = NULL;
enum _P2PSERVICESTATUS mLocalStatus = STATUS_UNONLINE;
#define P2PHOST_IP  "192.168.179.2"
#define P2PHOST_PORT	12990
#define SUPPORT_P2PCLIENT
#define SENDCOMAND_TIMEOUT  1000
#define	DEFAULT_USERID	"111111"
#define	DEFAULT_PASSWD	"222222"
#define	DEFAULT_USERNAME    "vinsion"

#ifdef SUPPORT_P2PSERVER
#define P2PLOCAL_DEFAULTTYPE  P2PDEVICE_SERVER
#else
#define P2PLOCAL_DEFAULTTYPE  P2PDEVICE_USER
#endif

typedef struct _P2PServiceCmdDeal{
    int Cmd;
    int (*DealProc)(NetPacket *NetP, unsigned long lParam);
}P2PServiceCmdDeal;

static int P2PCreateProc(NetPacket *NetP, unsigned long lParam)
{
#ifdef SUPPORT_P2PSERVER
    GFP2PService_LOG_DEBUG("--------\n");
    if(NetP->Size == sizeof(SendMsgP2PServiceCreate))
    {
	SendMsgP2PServiceCreate *Packet = (SendMsgP2PServiceCreate *)NetP->Buf;
	RecvMsgP2PServiceCreate *sPacket = (RecvMsgP2PServiceCreate *)malloc(sizeof(RecvMsgP2PServiceCreate));
	memset(sPacket, 0, sizeof(RecvMsgP2PServiceCreate));
	sPacket->Id = gf_thrd_get_tick();
	sPacket->Cmd = CMD_P2PCREATE_RESPOND;
	sPacket->Type = P2PLOCAL_DEFAULTTYPE;
	sPacket->Size = sizeof(RecvMsgP2PServiceCreate);
	if(Packet->CrcSum != GfGetCRC32(NetP->Buf, NetP->Size))
	{
	    sPacket->Result = ERROR_CRCSUM;
	    return GfUdpClientSend(mPClient, NetP->Ip, NetP->Port, (char *)&sPacket, sizeof(RecvMsgP2PServiceCreate), SENDCOMAND_TIMEOUT);
	}
	memcpy(sPacket->UserId, Packet->UserId, sizeof(sPacket->UserId);
	memcpy(sPacket->UserName, Packet->UserName, sizeof(sPacket->UserName);
	GetContextMD5Ex(Packet->PassWd, sizeof(PassWd), sPacket->PassWd, sizeof(sPacket->PassWd));
	GetUuidEx(sPacket->Uuid, sizeof(sPacket->Uuid));
	return GfUdpClientSend(mPClient, NetP->Ip, NetP->Port, (char *)&sPacket, sizeof(RecvMsgP2PServiceCreate), SENDCOMAND_TIMEOUT);
    }
#elif	defined(SUPPORT_P2PCLIENT)
    GFP2PService_LOG_DEBUG("--------\n");
#endif
    return 0;
}

static int P2PLoginProc(NetPacket *NetP, unsigned long lParam)
{
#ifdef SUPPORT_P2PSERVER
    GFP2PService_LOG_DEBUG("--------\n");
#elif	defined(SUPPORT_P2PCLIENT)
    GFP2PService_LOG_DEBUG("--------\n");
#endif
    return 0;
}

static int P2PLogoutProc(NetPacket *NetP, unsigned long lParam)
{
#ifdef SUPPORT_P2PSERVER
    GFP2PService_LOG_DEBUG("--------\n");
#elif	defined(SUPPORT_P2PCLIENT)
    GFP2PService_LOG_DEBUG("--------\n");
#endif
    return 0;
}

static int P2PDetectProc(NetPacket *NetP, unsigned long lParam)
{
#ifdef SUPPORT_P2PSERVER
    GFP2PService_LOG_DEBUG("--------\n");
#elif	defined(SUPPORT_P2PCLIENT)
    GFP2PService_LOG_DEBUG("--------\n");
#endif
    return 0;
}

static int P2POpenP2PProc(NetPacket *NetP, unsigned long lParam)
{
#ifdef SUPPORT_P2PSERVER
    GFP2PService_LOG_DEBUG("--------\n");
#elif	defined(SUPPORT_P2PCLIENT)
    GFP2PService_LOG_DEBUG("--------\n");
#endif
    return 0;
}

static int P2POpenPROXYProc(NetPacket *NetP, unsigned long lParam)
{
#ifdef SUPPORT_P2PSERVER
    GFP2PService_LOG_DEBUG("--------\n");
#elif	defined(SUPPORT_P2PCLIENT)
    GFP2PService_LOG_DEBUG("--------\n");
#endif
    return 0;
}

static int P2PHeartProc(NetPacket *NetP, unsigned long lParam)
{
#ifdef SUPPORT_P2PSERVER
    GFP2PService_LOG_DEBUG("--------\n");
#elif	defined(SUPPORT_P2PCLIENT)
    GFP2PService_LOG_DEBUG("--------\n");
#endif
    return 0;
}

#ifdef SUPPORT_P2PSERVER
//Server
static P2PServiceCmdDeal mCmdDealProc[] = {
    {CMD_P2PCREATE_REQUEST, P2PCreateProc},
    {CMD_P2PLOGIN_REQUEST, P2PLoginProc},
    {CMD_P2PLOGOUT_REQUEST, P2PLogoutProc},
    {CMD_P2PDETECT_REQUEST, P2PDetectProc},
    {CMD_P2POPENP2P_REQUEST, P2POpenPROXYProc},
    {CMD_P2POPENPROXY_REQUEST, P2POpenPROXYProc},
    {CMD_P2PHEARTPACKET, P2PHeartProc},
};
#elif	defined(SUPPORT_P2PCLIENT)
//Client
static P2PServiceCmdDeal mCmdDealProc[] = {
    {CMD_P2PCREATE_RESPOND, P2PCreateProc},
    {CMD_P2PLOGIN_RESPOND, P2PLoginProc},
    {CMD_P2PLOGOUT_RESPOND, P2PLogoutProc},
    {CMD_P2PDETECT_RESPOND, P2PDetectProc},
    {CMD_P2POPENP2P_RESPOND, P2POpenP2PProc},
    {CMD_P2POPENPROXY_RESPOND, P2POpenPROXYProc},
};

//CMD_P2PCREATE_REQUEST
int P2PSendCreate(void)
{
    SendMsgP2PServiceCreate Data = {0};
    Data.Head.Id = gf_thrd_get_tick();
    Data.Head.Cmd = CMD_P2PCREATE_REQUEST;
    Data.Head.Type = P2PLOCAL_DEFAULTTYPE;
    Data.Head.Size = sizeof(SendMsgP2PServiceCreate);
    
    snprintf(Data.UserId, sizeof(Data.UserId), "%s", DEFAULT_USERID);
    snprintf(Data.PassWd, sizeof(Data.PassWd), "%s", DEFAULT_PASSWD);
    snprintf(Data.UserName, sizeof(Data.UserName), "%s", DEFAULT_USERNAME);

    Data.Head.CrcSum = GfGetCRC32((char *)(&Data + sizeof(MsgP2PServiceHead)), sizeof(SendMsgP2PServiceCreate) - sizeof(MsgP2PServiceHead));
    return GfUdpClientSend(mPClient, mPClient->HostIp, mPClient->HostPort, (char *)&Data, sizeof(SendMsgP2PServiceCreate), SENDCOMAND_TIMEOUT);
}

//CMD_P2PLOGIN_REQUEST
int P2PSendLogin(void)
{
    SendMsgP2PServiceLogin Data = {0};
    Data.Head.Id = gf_thrd_get_tick();
    Data.Head.Cmd = CMD_P2PLOGIN_REQUEST;
    Data.Head.Type = P2PLOCAL_DEFAULTTYPE;
    Data.Head.Size = sizeof(SendMsgP2PServiceLogin);
    char PassWd[32] = {0};
    //未注册
    if(0 > GfParamerterGet("Uuid", Data.Uuid, sizeof(Data.Uuid)) || 0 > GfParamerterGet("UserId", Data.UserId, sizeof(Data.UserId)) || 0 > GfParamerterGet("PassWd", PassWd, sizeof(PassWd)))
	return -2;
    GetContextMD5Ex(PassWd, strlen(PassWd), Data.PassWd, sizeof(Data.PassWd));

    Data.Head.CrcSum = GfGetCRC32((char *)(&Data + sizeof(MsgP2PServiceHead)), sizeof(SendMsgP2PServiceLogin) - sizeof(MsgP2PServiceHead));
    return GfUdpClientSend(mPClient, mPClient->HostIp, mPClient->HostPort, (char *)&Data, sizeof(SendMsgP2PServiceLogin), SENDCOMAND_TIMEOUT);
}

//CMD_P2PLOGOUT_REQUEST
int P2PSendLogout(void)
{
    SendMsgP2PServiceLogout Data = {0};
    Data.Head.Id = gf_thrd_get_tick();
    Data.Head.Cmd = CMD_P2PLOGOUT_REQUEST;
    Data.Head.Type = P2PLOCAL_DEFAULTTYPE;
    Data.Head.Size = sizeof(SendMsgP2PServiceLogout);
    char PassWd[32] = {0};
    //未注册
    if(0 > GfParamerterGet("Uuid", Data.Uuid, sizeof(Data.Uuid)) || 0 > GfParamerterGet("UserId", Data.UserId, sizeof(Data.UserId)) || 0 > GfParamerterGet("PassWd", PassWd, sizeof(PassWd)))
	return -2;
    GetContextMD5Ex(PassWd, strlen(PassWd), Data.PassWd, sizeof(Data.PassWd));

    Data.Head.CrcSum = GfGetCRC32((char *)(&Data + sizeof(MsgP2PServiceHead)), sizeof(SendMsgP2PServiceLogout) - sizeof(MsgP2PServiceHead));
    return GfUdpClientSend(mPClient, mPClient->HostIp, mPClient->HostPort, (char *)&Data, sizeof(SendMsgP2PServiceLogout), SENDCOMAND_TIMEOUT);
}

//CMD_P2PDETECT_REQUEST
int P2PSendDetect(char *NewPassWd, char *NewUserName)
{
    //格式错误
    if(!NewPassWd && !NewUserName)
	return -1;
    if(NewPassWd && strlen(NewPassWd) < 8)
	return -1;
    if(NewUserName && strlen(NewUserName) < 1)
	return -1;

    SendMsgP2PServiceDetect Data = {0};
    Data.Head.Id = gf_thrd_get_tick();
    Data.Head.Cmd = CMD_P2PDETECT_REQUEST;
    Data.Head.Type = P2PLOCAL_DEFAULTTYPE;
    Data.Head.Size = sizeof(SendMsgP2PServiceDetect);
    char PassWd[32] = {0};
    //未注册
    if(0 > GfParamerterGet("Uuid", Data.Uuid, sizeof(Data.Uuid)) || 0 > GfParamerterGet("UserId", Data.UserId, sizeof(Data.UserId)) || 0 > GfParamerterGet("PassWd", PassWd, sizeof(PassWd)))
	return -2;
    GetContextMD5Ex(PassWd, strlen(PassWd), Data.PassWd, sizeof(Data.PassWd));

    if(NewPassWd)
	GetContextMD5Ex(NewPassWd, strlen(NewPassWd), Data.NewPassWd, sizeof(Data.NewPassWd));
    if(NewUserName)
	snprintf(Data.NewUserName, sizeof(Data.NewUserName), "%s", NewUserName);

    Data.Head.CrcSum = GfGetCRC32((char *)(&Data + sizeof(MsgP2PServiceHead)), sizeof(SendMsgP2PServiceDetect) - sizeof(MsgP2PServiceHead));
    return GfUdpClientSend(mPClient, mPClient->HostIp, mPClient->HostPort, (char *)&Data, sizeof(SendMsgP2PServiceDetect), SENDCOMAND_TIMEOUT);
}

//CMD_P2POPENP2P_REQUEST
int P2PSendOpenP2P(void)
{
    SendMsgP2PServiceOpenP2P Data = {0};
    Data.Head.Id = gf_thrd_get_tick();
    Data.Head.Cmd = CMD_P2POPENP2P_REQUEST;
    Data.Head.Type = P2PLOCAL_DEFAULTTYPE;
    Data.Head.Size = sizeof(SendMsgP2PServiceOpenP2P);
    char PassWd[32] = {0};
    //未注册
    if(0 > GfParamerterGet("Uuid", Data.Uuid, sizeof(Data.Uuid)) || 0 > GfParamerterGet("UserId", Data.UserId, sizeof(Data.UserId)) || 0 > GfParamerterGet("PassWd", PassWd, sizeof(PassWd)))
	return -2;
    GetContextMD5Ex(PassWd, strlen(PassWd), Data.PassWd, sizeof(Data.PassWd));

    Data.Head.CrcSum = GfGetCRC32((char *)(&Data + sizeof(MsgP2PServiceHead)), sizeof(SendMsgP2PServiceOpenP2P) - sizeof(MsgP2PServiceHead));
    return GfUdpClientSend(mPClient, mPClient->HostIp, mPClient->HostPort, (char *)&Data, sizeof(SendMsgP2PServiceOpenP2P), SENDCOMAND_TIMEOUT);
}

//CMD_P2POPENPROXY_REQUEST
int P2PSendOpenPROXY(void)
{
    SendMsgP2PServiceOpenP2P Data = {0};
    Data.Head.Id = gf_thrd_get_tick();
    Data.Head.Cmd = CMD_P2POPENPROXY_REQUEST;
    Data.Head.Type = P2PLOCAL_DEFAULTTYPE;
    Data.Head.Size = sizeof(SendMsgP2PServiceOpenPROXY);
    char PassWd[32] = {0};
    //未注册
    if(0 > GfParamerterGet("Uuid", Data.Uuid, sizeof(Data.Uuid)) || 0 > GfParamerterGet("UserId", Data.UserId, sizeof(Data.UserId)) || 0 > GfParamerterGet("PassWd", PassWd, sizeof(PassWd)))
	return -2;
    GetContextMD5Ex(PassWd, strlen(PassWd), Data.PassWd, sizeof(Data.PassWd));

    Data.Head.CrcSum = GfGetCRC32((char *)(&Data + sizeof(MsgP2PServiceHead)), sizeof(SendMsgP2PServiceOpenPROXY) - sizeof(MsgP2PServiceHead));
    return GfUdpClientSend(mPClient, mPClient->HostIp, mPClient->HostPort, (char *)&Data, sizeof(SendMsgP2PServiceOpenPROXY), SENDCOMAND_TIMEOUT);
}

//CMD_P2PHEARTPACKET
int P2PSendHeartPacket(void)
{
    SendMsgP2PServiceHeartPactet Data = {0};
    Data.Head.Id = gf_thrd_get_tick();
    Data.Head.Cmd = CMD_P2PHEARTPACKET;
    Data.Head.Type = P2PLOCAL_DEFAULTTYPE;
    Data.Head.Size = sizeof(SendMsgP2PServiceHeartPactet);
    char PassWd[32] = {0};
    //未注册
    if(0 > GfParamerterGet("Uuid", Data.Uuid, sizeof(Data.Uuid)) || 0 > GfParamerterGet("UserId", Data.UserId, sizeof(Data.UserId)) || 0 > GfParamerterGet("PassWd", PassWd, sizeof(PassWd)))
	return -2;
    GetContextMD5Ex(PassWd, strlen(PassWd), Data.PassWd, sizeof(Data.PassWd));
    Data.Status = mLocalStatus;

    Data.Head.CrcSum = GfGetCRC32((char *)(&Data + sizeof(MsgP2PServiceHead)), sizeof(SendMsgP2PServiceHeartPactet) - sizeof(MsgP2PServiceHead));
    return GfUdpClientSend(mPClient, mPClient->HostIp, mPClient->HostPort, (char *)&Data, sizeof(SendMsgP2PServiceHeartPactet), SENDCOMAND_TIMEOUT);
}
#endif

static int P2PServiceRecv(NetPacket *NetP, unsigned long lParam)
{
    struct in_addr in;
    in.s_addr = NetP->Ip; // it’s net longvalue;
    GFP2PService_LOG_DEBUG("[%s:%d]----size:[%d]--->buf: [%s]\n", inet_ntoa(in), ntohs(NetP->Port), NetP->Size, NetP->Buf);

    if(sizeof(MsgP2PServiceHead) > NetP->Size)
    {
	printf("Invalid Packet------------\n");
	return 0;
    }

    MsgP2PServiceHead *Head = (MsgP2PServiceHead *)NetP->Buf;
    unsigned int CheckCrc = GfGetCRC32(NetP->Buf + sizeof(MsgP2PServiceHead), NetP->Size - sizeof(MsgP2PServiceHead));
    if(Head->CrcSum != CheckCrc)
    {
	GFP2PService_LOG_DEBUG("CheckCrc Error [%08x]---[%08x]\n", Head->CrcSum, CheckCrc);
	return 0;
    }

    int i = 0;
    for(i = 0; i < sizeof(mCmdDealProc); i++)
    {
	if(mCmdDealProc[i].Cmd = Head->Cmd)
	{
	    mCmdDealProc[i].DealProc(NetP, lParam);
	    return 0;
	}
    }
    GFP2PService_LOG_DEBUG("Invalid Packet\n");
    return 0;
}

static int P2PServiceError(NetPacket *NetP, unsigned long lParam)
{
    struct in_addr in;
    in.s_addr = NetP->Ip; // it’s net longvalue;
    GFP2PService_LOG_DEBUG("[%s:%d]------\n", inet_ntoa(in), ntohs(NetP->Port));
    return 0;
}

static int ParamInit(SUdpClient *P)
{
    if(!P)
	return -1;
    memset(P, 0, sizeof(SUdpClient));
    P->LocalIp = INADDR_ANY;
    P->LocalPort = htons(P2PHOST_PORT);
    P->HostIp = inet_addr(P2PHOST_IP);
    P->HostPort = htons(P2PHOST_PORT);
    P->pPacketHandler = P2PServiceRecv;
    P->pErrorHandler = P2PServiceError;
    return 0;
}

int GfP2PServiceInit(void)
{
    if(!mPClient)
	mPClient = (SUdpClient *)malloc(sizeof(SUdpClient));
    if(!mPClient)
	return -1;
    ParamInit(mPClient);
    GfUdpClientInit(mPClient);
}

int GfP2PServiceExit(void)
{
    if(!mPClient)
	return -1;
    if(!GfUdpClientIsInit)
	GfUdpClientExit(mPClient);
    free(mPClient);
    mPClient = NULL;
    return 0;
}
