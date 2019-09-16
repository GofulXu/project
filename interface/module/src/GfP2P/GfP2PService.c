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
#include "gfparamerter.h"
#include "gfreportlog.h"
#include "GfCRC32.h"
#include "GfP2PService.h"
#include "uuid/uuid.h"

SUdpClient *mPClient = NULL;
#define P2PHOST_IP  "192.168.179.2"
#define P2PHOST_PORT	12990


static int P2PServiceRecv(SOCKET Socket, unsigned long Ip, unsigned short Port, char *Buf, int Size, unsigned long lParam)
{
    struct in_addr in;
    in.s_addr = Ip; // it’s net longvalue;
    GFP2PService_LOG_DEBUG("[%s:%d]----size:[%d]--->buf: [%s]\n", inet_ntoa(in), ntohs(Port), Size, Buf);

    if(sizeof(MsgP2PServiceHead) < Size)
    {
	printf("Invalid Packet------------\n");
	return 0;
    }

    MsgP2PServiceHead *Head = (MsgP2PServiceHead *)Buf;
    unsigned int CheckCrc = GfGetCRC32(Buf + sizeof(MsgP2PServiceHead), Size - sizeof(MsgP2PServiceHead));
    if(Head->CrcSum != CheckCrc)
    {
	GFP2PService_LOG_DEBUG("CheckCrc Error [%08x]---[%08x]\n", Head->CrcSum, CheckCrc);
    }


    return 0;
}

static int P2PServiceError(SOCKET socket, unsigned long ip, unsigned short port, unsigned long lParam)
{
    struct in_addr in;
    in.s_addr = ip; // it’s net longvalue;
    GFP2PService_LOG_DEBUG("[%s:%d]------\n", inet_ntoa(in), ntohs(port));
    return 0;
}

int ParamInit(SUdpClient *P)
{
    if(!P)
	return -1;
    memset(P, 0, sizeof(SUdpClient));
    P->localip = INADDR_ANY;
    P->localport = htons(P2PHOST_PORT);
    P->hostip = inet_addr(P2PHOST_IP);
    P->hostport = htons(P2PHOST_PORT);
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
    GfUdpClientExit(mPClient);
    free(mPClient);
    mPClient = NULL;
    return 0;
}
