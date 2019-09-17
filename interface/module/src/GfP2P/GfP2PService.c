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
#define P2PHOST_IP  "192.168.179.2"
#define P2PHOST_PORT	12990
#define SUPPORT_P2PSERVER
#define SENDCOMAND_TIMEOUT  1000
#define	DEFAULT_USERID	"111111"
#define	DEFAULT_PASSWD	"222222"
#define	DEFAULT_USERNAME    "vinsion"

typedef struct _P2PServiceCmdDeal{
    int Cmd;
    int (*DealProc)(NetPacket *NetP, unsigned long lParam);
}P2PServiceCmdDeal;

static int P2PCreateProc(NetPacket *NetP, unsigned long lParam)
{
#ifdef SUPPORT_P2PSERVER
    GFP2PService_LOG_DEBUG("--------\n");
#elif	SUPPORT_P2PCLIENT
    GFP2PService_LOG_DEBUG("--------\n");
#endif
    return 0;
}

static int P2PLoginProc(NetPacket *NetP, unsigned long lParam)
{
#ifdef SUPPORT_P2PSERVER
    GFP2PService_LOG_DEBUG("--------\n");
#elif	SUPPORT_P2PCLIENT
    GFP2PService_LOG_DEBUG("--------\n");
#endif
    return 0;
}

static int P2PLogoutProc(NetPacket *NetP, unsigned long lParam)
{
#ifdef SUPPORT_P2PSERVER
    GFP2PService_LOG_DEBUG("--------\n");
#elif	SUPPORT_P2PCLIENT
    GFP2PService_LOG_DEBUG("--------\n");
#endif
    return 0;
}

static int P2PDetectProc(NetPacket *NetP, unsigned long lParam)
{
#ifdef SUPPORT_P2PSERVER
    GFP2PService_LOG_DEBUG("--------\n");
#elif	SUPPORT_P2PCLIENT
    GFP2PService_LOG_DEBUG("--------\n");
#endif
    return 0;
}

static int P2POpenP2pProc(NetPacket *NetP, unsigned long lParam)
{
#ifdef SUPPORT_P2PSERVER
    GFP2PService_LOG_DEBUG("--------\n");
#elif	SUPPORT_P2PCLIENT
    GFP2PService_LOG_DEBUG("--------\n");
#endif
    return 0;
}

static int P2POpenProxyProc(NetPacket *NetP, unsigned long lParam)
{
#ifdef SUPPORT_P2PSERVER
    GFP2PService_LOG_DEBUG("--------\n");
#elif	SUPPORT_P2PCLIENT
    GFP2PService_LOG_DEBUG("--------\n");
#endif
    return 0;
}

static int P2PHeartProc(NetPacket *NetP, unsigned long lParam)
{
#ifdef SUPPORT_P2PSERVER
    GFP2PService_LOG_DEBUG("--------\n");
#elif	SUPPORT_P2PCLIENT
    GFP2PService_LOG_DEBUG("--------\n");
#endif
    return 0;
}

#ifdef SUPPORT_P2PSERVER
//Server
static P2PServiceCmdDeal CmdDealProc[] = {
    {CMD_P2PCREATE_REQUEST, P2PCreateProc},
    {CMD_P2PLOGIN_REQUEST, P2PLoginProc},
    {CMD_P2PLOGOUT_REQUEST, P2PLogoutProc},
    {CMD_P2PDETECT_REQUEST, P2PDetectProc},
    {CMD_P2POPENP2P_REQUEST, P2POpenProxyProc},
    {CMD_P2POPENPROXY_REQUEST, P2POpenProxyProc},
    {CMD_P2PHEARTPACKET, P2PHeartProc},
};
#elif SUPPORT_P2PCLIENT
//Client
static P2PServiceCmdDeal CmdDealProc[] = {
    {CMD_P2PCREATE_RESPOND, P2PCreateProc},
    {CMD_P2PLOGIN_RESPOND, P2PLoginProc},
    {CMD_P2PLOGOUT_RESPOND, P2PLogoutProc},
    {CMD_P2PDETECT_RESPOND, P2PDetectProc},
    {CMD_P2POPENP2P_RESPOND, P2POpenP2pProc},
    {CMD_P2POPENPROXY_RESPOND, P2POpenProxyProc},
};

int P2PSendCreate(void)
{
    SendMsgP2PServiceCreate Data = {0};
    Data.Id = gf_thrd_get_tick();
    Data.Cmd = CMD_P2PCREATE_REQUEST;
    Data.Type = P2PDEVICE_USER;
    Data.Size = sizeof(SendMsgP2PServiceCreate);
    
    snprintf(Data.UserID, sizeof(Data.UserID), "%s", DEFAULT_USERID);
    snprintf(Data.Passwd, sizeof(Data.Passwd), "%s", DEFAULT_PASSWD);
    snprintf(Data.UserName, sizeof(Data.UserName), "%s", DEFAULT_USERNAME);

    Data.CrcSum = GfGetCRC32(&Data + sizeof(MsgP2PServiceHead), sizeof(SendMsgP2PServiceCreate) - sizeof(MsgP2PServiceHead));
    return GfUdpClientSend(mPClient, mPClient->HostIp, mPClient->HostPort, (char *)&Data, sizeof(SendMsgP2PServiceCreate), SENDCOMAND_TIMEOUT);
}

int P2PSendCreate(void)
{
    SendMsgP2PServiceCreate Data = {0};
    Data.Id = gf_thrd_get_tick();
    Data.Cmd = CMD_P2PCREATE_REQUEST;
    Data.Type = P2PDEVICE_USER;
    Data.Size = sizeof(SendMsgP2PServiceCreate);
    
    snprintf(Data.UserID, sizeof(Data.UserID), "%s", DEFAULT_USERID);
    snprintf(Data.Passwd, sizeof(Data.Passwd), "%s", DEFAULT_PASSWD);
    snprintf(Data.UserName, sizeof(Data.UserName), "%s", DEFAULT_USERNAME);

    Data.CrcSum = GfGetCRC32(&Data + sizeof(MsgP2PServiceHead), sizeof(SendMsgP2PServiceCreate) - sizeof(MsgP2PServiceHead));
    return GfUdpClientSend(mPClient, mPClient->HostIp, mPClient->HostPort, (char *)&Data, sizeof(SendMsgP2PServiceCreate), SENDCOMAND_TIMEOUT);
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
    for(i = 0; i < sizeof(CmdDealProc); i++)
    {
	if(CmdDealProc[i].Cmd = Head->Cmd)
	{
	    CmdDealProc[i].DealProc(NetP, lParam);
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
