/*************************************************************************
	> File Name: include/GfP2PService.h
	> Author: vinsion
	> Mail: vinsion@vinsion.mail 
	> Created Time: Mon 16 Sep 2019 12:12:29 PM +06
 ************************************************************************/

#ifndef	__GFP2PSERVICE_H__
#define __GFP2PSERVICE_H__

#define GFP2PService_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "P2PService", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFP2PService_LOG_INFO( format, ...)		gf_log(LOG_LEVEL_INFO, "P2PService", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFP2PService_LOG_WARN( format, ...)		gf_log(LOG_LEVEL_WARN, "P2PService", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFP2PService_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "P2PService", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFP2PService_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "P2PService", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

//每次发送请求必须带密码盒UUID, 增加安全性
enum _P2PSERVICECMD{
    CMD_P2PCREATE_REQUEST = 0x80,
    CMD_P2PCREATE_RESPOND,
    CMD_P2PLOGIN_REQUEST,
    CMD_P2PLOGIN_RESPOND,
    CMD_P2PLOGOUT_REQUEST,
    CMD_P2PLOGOUT_RESPOND,
    CMD_P2PDETECT_REQUEST,
    CMD_P2PDETECT_RESPOND,
    CMD_P2POPENP2P_REQUEST,
    CMD_P2POPENP2P_RESPOND,
    CMD_P2POPENPROXY_REQUEST,
    CMD_P2POPENPROXY_RESPOND,
    CMD_P2PHEARTPACKET
};

enum _P2PSERVICESTATUS{
    STATUS_ONLINE,	    //在线
    STATUS_UNONLINE,	    //不在线
    STATUS_MISSED,	    //失去联系（暂时掉线）
    STATUS_P2PCONNECT,	    //P2P已连接
    STATUS_P2PDISCONNECT,   //未连接
    STATUS_PROXYCONNECT,    //PROXY已连接
    STATUS_PROXYDISCONNECT  //PROXY未连接
};

typedef	struct _MsgP2PServiceHead{
    unsigned int Id;	    //timetick时间戳
    unsigned int Cmd;	    //命令字
    unsigned int Type;	    //设备类型
    unsigned int Size;	    //整包大小
    unsigned int CrcSum;    //校验码
}MsgP2PServiceHead;


//CMD_P2PCREATE_REQUEST
typedef struct _SendMsgP2PServiceCreate{
    char UserID[32];	
    char Passwd[32];	//+md5加密
    char UserName[32];	//预留
}SendMsgP2PServiceCreate;

//CMD_P2PCREATE_RESPOND
typedef struct _RecvMsgP2PServiceCreate{
    char UserID[32];	
    char Passwd[32];	//+md5加密
    char UserName[32];	//预留
    char UUID[32];	//用户端注册时服务器返回
    unsigned int result;    //suc=1, fail=2~5
}RecvMsgP2PServiceCreate;

//CMD_P2PLOGIN_REQUEST
typedef struct _SendMsgP2PServiceLogin{
    char UserID[32];	
    char Passwd[32];	//+md5加密
    char UserName[32];	//预留
    char UUID[32];	//用户端注册时服务器返回
}SendMsgP2PServiceLogin;

//CMD_P2PLOGIN_RESPOND
typedef struct _RecvMsgP2PServiceLogin{
    char UserID[32];	
    char UserName[32];	//预留
    char UUID[32];	//用户端注册时服务器返回
    unsigned int Result;    //suc=1, fail=2~5
}RecvMsgP2PServiceLogin;

//CMD_P2PLOGOUT_REQUEST
typedef struct _SendMsgP2PServiceLogout{
    char UserID[32];	
    char Passwd[32];	//+md5加密
    char UserName[32];	//预留
    char UUID[32];	//用户端注册时服务器返回
}SendMsgP2PServiceLogout;

//CMD_P2PLOGOUT_RESPOND
typedef struct _RecvMsgP2PServiceLogout{
    char UserID[32];	
    char UserName[32];	//预留
    char UUID[32];	//用户端注册时服务器返回
    unsigned int Result;    //suc=1, fail=2~5
}RecvMsgP2PServiceLogout;

//CMD_P2PDETECT_REQUEST
typedef struct _SendMsgP2PServiceDetect{
    char UserID[32];	
    char Passwd[32];	//+md5加密
    char UserName[32];	//预留
    char UUID[32];	//用户端注册时服务器返回
    char NewPasswd[32];
    char NewUserName[32];
}SendMsgP2PServiceDetect;

//CMD_P2PDETECT_RESPOND
typedef struct _RecvMsgP2PServiceDetect{
    char UserID[32];	
    char Passwd[32];	//+md5加密---new
    char UserName[32];	//预留--new
    char UUID[32];	//用户端注册时服务器返回
    unsigned int Result;    //suc=1, fail=2~5
}RecvMsgP2PServiceDetect;

//CMD_P2POPENP2P_REQUEST
typedef struct _SendMsgP2PServiceOpenP2P{
    char UserID[32];	
    char Passwd[32];	//+md5加密
    char UserName[32];	//预留
    char UUID[32];	//用户端注册时服务器返回
}SendMsgP2PServiceOpenP2P;

typedef struct _DevInfo{
    char UUID[32];
    char Ip[32];
    unsigned short Port;
    unsigned int Type;
}DevInfo;

//CMD_P2POPENP2P_RESPOND, 设备端接收到设备信息后立刻进行P2P连接，同时反馈结果以及相应时间戳到服务器
typedef struct _RecvMsgP2PServiceOpenP2P{
    char UserID[32];	
    char UserName[32];	//预留
    char UUID[32];	//用户端注册时服务器返回
    DevInfo Dev;	//对接设备信息
    unsigned TimeTick;	//返回时间戳
    unsigned int Result;    //suc=1, fail=2~5
}RecvMsgP2PServiceOpenP2P;

//CMD_P2POPENPROXY_REQUEST  //服务器收到后检测设备端是否在线，在线的话将用户加入hash表做代理透传服务
typedef struct _SendMsgP2PServiceOpenPROXY{
    char UserID[32];	
    char Passwd[32];	//+md5加密
    char UserName[32];	//预留
    char UUID[32];	//用户端注册时服务器返回
}SendMsgP2PServiceOpenPROXY;

//CMD_P2POPENPROXY_RESPOND
typedef struct _RecvMsgP2PServiceOpenPROXY{
    char UserID[32];	
    char UserName[32];	//预留
    char UUID[32];	//用户端注册时服务器返回
    unsigned int Result;    //suc=1, fail=2~5
}RecvMsgP2PServiceOpenPROXY;

//CMD_P2PHEARTPACKET
typedef struct _SendMsgP2PServiceHeartPacket{
    char UserID[32];	
    char UserName[32];	//预留
    char UUID[32];	//用户端注册时服务器返回
    enum _P2PSERVICESTATUS Status;
}SendMsgP2PServiceHeartPactet;


int GfP2PServiceInit(void);
int GfP2PServiceExit(void);
#endif /*__GFP2PSERVICE_H__*/
