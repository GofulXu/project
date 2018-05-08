#ifndef __IOT_DEFINE_H__
#define __IOT_DEFINE_H__


#define MTYPE_TCP	1
#define MTYPE_UDP	2
#define M_DEVTYPE	1
#define M_DEVID		2
#define M_USERID	601001
#define M_USERNAME	"goeful"
#define M_PASSWD	"goeful123.."


typedef enum _m_cmdid_t{
	CMD_HEART = 101,
	CMD_CREATE,
	CMD_LOGIN,
	CMD_GET,
	CMD_POST,
	CMD_CHANGE_USERPWD
}m_cmdid_t;


#endif /*__IOT_DEFINE_H__*/
