/** 
 * @file gfevent.h
 * @brief  参考SDL event定义事件数据结构
 * @author chenkai
 * @date 2010-12-10 created
 * @update:yihong
 */
#ifndef __GFEVENT_H__
#define __GFEVENT_H__

#include "gftype.h"
#include "gfkey.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @name General keyboard/mouse state definitions */
#define GF_RELEASED	0
#define GF_PRESSED	1

/** Event enumerations */
typedef enum 
{
	GF_NOEVENT = 0,			/**< Unused (do not remove) */
	GF_RCKEYDOWN,			/**< Remote control key down */
	GF_RCKEYUP,				/**< Remote control key up */
	GF_KBKEYDOWN,			/**< Keys pressed */
	GF_KBKEYUP,				/**< Keys released */
	GF_MOUSEMOTION,			/**< Mouse moved */
	GF_MOUSEBUTTONDOWN,		/**< Mouse button pressed */
	GF_MOUSEBUTTONUP,		/**< Mouse button released */
	GF_JOYAXISMOTION,		/**< Joystick axis motion */
	GF_JOYBALLMOTION,		/**< Joystick trackball motion */
	GF_JOYHATMOTION,		/**< Joystick hat position change */
	GF_JOYBUTTONDOWN,		/**< Joystick button pressed */
	GF_JOYBUTTONUP,			/**< Joystick button released */
	GF_SYSTEM_EVENT,		/**< System event,SLEEP,WAKEUP,REBOOT,UPGRADE,QUIT*/
	GF_NETWORK_EVENT,		/**< Network event*/
	GF_HARDWARE_EVENT,		/**< Hardware event*/
	GF_MEDIA_EVENT,			/**多媒体事件*/
	GF_BROWSER_EVENT,		/**浏览器事件*/	
	GF_NTP_EVENT,			/**ntp 同步事件*/
	GF_HEARTBEAT_EVENT,		/**10ms的心跳事件*/
	GF_CWMP_EVENT,			/**终端网管事件 */
	GF_EVENT_RESERVED3,		/**< Reserved for future use.. */
	GF_EVENT_RESERVED4,		/**< Reserved for future use.. */
	GF_EVENT_RESERVED5,		/**< Reserved for future use.. */
	GF_USEREVENT = 24,		
	/** Events GF_USEREVENT through GF_MAXEVENTS-1 are for your use */
	/** This last event is only for bounding internal arrays
	 *  It is the number of bits in the event mask datatype -- Uint32
	 */
	GF_NUMEVENTS = 32
} gf_eventtype_t;

/** @name Predefined event masks */
#define GF_EVENTMASK(X)	(1<<(X))
typedef enum 
{
	GF_RCKEYDOWNMASK = GF_EVENTMASK(GF_RCKEYDOWN),
	GF_RCKEYUPMASK	 = GF_EVENTMASK(GF_RCKEYUP),
	GF_RCKEYEVENTMASK = GF_EVENTMASK(GF_RCKEYDOWN)|
		GF_EVENTMASK(GF_RCKEYUP),
	GF_KBKEYDOWNMASK = GF_EVENTMASK(GF_KBKEYDOWN),
	GF_KBKEYUPMASK	= 	GF_EVENTMASK(GF_KBKEYUP),
	GF_KBKEYEVENTMASK = GF_EVENTMASK(GF_KBKEYDOWN)|
		GF_EVENTMASK(GF_KBKEYUP),
	GF_KEYEVENTMASK	= 	GF_EVENTMASK(GF_KBKEYDOWN)|
		GF_EVENTMASK(GF_KBKEYUP)|
		GF_EVENTMASK(GF_RCKEYDOWN)|
		GF_EVENTMASK(GF_RCKEYUP),
	GF_MOUSEMOTIONMASK	= GF_EVENTMASK(GF_MOUSEMOTION),
	GF_MOUSEBUTTONDOWNMASK	= GF_EVENTMASK(GF_MOUSEBUTTONDOWN),
	GF_MOUSEBUTTONUPMASK	= GF_EVENTMASK(GF_MOUSEBUTTONUP),
	GF_MOUSEEVENTMASK	= GF_EVENTMASK(GF_MOUSEMOTION)|
		GF_EVENTMASK(GF_MOUSEBUTTONDOWN)|
		GF_EVENTMASK(GF_MOUSEBUTTONUP),
	GF_JOYAXISMOTIONMASK	= GF_EVENTMASK(GF_JOYAXISMOTION),
	GF_JOYBALLMOTIONMASK	= GF_EVENTMASK(GF_JOYBALLMOTION),
	GF_JOYHATMOTIONMASK	= GF_EVENTMASK(GF_JOYHATMOTION),
	GF_JOYBUTTONDOWNMASK	= GF_EVENTMASK(GF_JOYBUTTONDOWN),
	GF_JOYBUTTONUPMASK	= GF_EVENTMASK(GF_JOYBUTTONUP),
	GF_JOYEVENTMASK	= GF_EVENTMASK(GF_JOYAXISMOTION)|
		GF_EVENTMASK(GF_JOYBALLMOTION)|
		GF_EVENTMASK(GF_JOYHATMOTION)|
		GF_EVENTMASK(GF_JOYBUTTONDOWN)|
		GF_EVENTMASK(GF_JOYBUTTONUP),
	GF_INPUTEVENTMASK = GF_KEYEVENTMASK|GF_MOUSEEVENTMASK|GF_JOYEVENTMASK,
	GF_SYSTEMEVENTMASK = GF_EVENTMASK(GF_SYSTEM_EVENT),
	GF_NETWORKEVENTMASK = GF_EVENTMASK(GF_NETWORK_EVENT),
	GF_HARDWAREEVENTMASK = GF_EVENTMASK(GF_HARDWARE_EVENT),
	GF_MEDIAEVENTMASK = GF_EVENTMASK(GF_MEDIA_EVENT),
	GF_BROWSEREVENTMASK = GF_EVENTMASK(GF_BROWSER_EVENT),
	GF_NTPEVENTMASK = GF_EVENTMASK(GF_NTP_EVENT),
	GF_HEARTBEATEVENTMASK = GF_EVENTMASK(GF_HEARTBEAT_EVENT),
	GF_CWMPEVENTMASK = GF_EVENTMASK(GF_CWMP_EVENT),
	GF_USEREVENTMASK = GF_EVENTMASK(GF_USEREVENT)
}gf_eventmask_t;

#define GF_ALLEVENTS		0xFFFFFFFF
/**Remotecontrol event structure*/
typedef struct _gf_rcevent
{
	uint8_t type;	 /**< GF_RCKEYDOWN or GF_RCKEYUP */
	uint8_t model;	 /**< 00 FrontPanel,FF 华为普通遥控器*/
	uint8_t state;	 /**GF_PRESSED or GF_RELEASED*/
	uint32_t  phycode; /**键盘扫描码*/ 
	gf_key_t kcode;
}gf_rcevent_t;

/** Keyboard event structure */
typedef struct _gf_kbevent 
{
	uint8_t type;	/**< GF_KBKEYDOWN or GF_KBKEYUP */
	uint8_t which;	/**< The keyboard device index */
	uint8_t state;	/**< GF_PRESSED or GF_RELEASED */
	uint8_t scancode; /**< 物理码*/ 
	gf_key_t kcode;
	gf_keymod_t kmod;
}gf_kbevent_t;

/** Mouse motion event structure */
typedef struct _gf_mousemotionevent 
{
	uint8_t type;	/**< GF_MOUSEMOTION */
	uint8_t which;	/**< The mouse device index */
	uint8_t state;	/**< The current button state */
	uint16_t x, y;	/**< The X/Y coordinates of the mouse */
	int16_t xrel;	/**< The relative motion in the X direction */
	int16_t yrel;	/**< The relative motion in the Y direction */
	int16_t wheelvalue; 
} gf_mousemotionevent_t;

/** Mouse button event structure */
typedef struct _gf_mousebuttonevent 
{
	uint8_t type;	/**< GF_MOUSEBUTTONDOWN or GF_MOUSEBUTTONUP */
	uint8_t which;	/**< The mouse device index */
	uint16_t button;	/**< The mouse button index */
	uint8_t state;	/**< GF_PRESSED or GF_RELEASED */
	uint16_t x, y;	/**< The X/Y coordinates of the mouse at press time */
} gf_mousebuttonevent_t;

/** Joystick axis motion event structure */
typedef struct _gf_joyaxisevent 
{
	uint8_t type;	/**< GF_JOYAXISMOTION */
	uint8_t which;	/**< The joystick device index */
	uint8_t axis;	/**< The joystick axis index */
	int16_t value;	/**< The axis value (range: -32768 to 32767) */
} gf_joyaxisevent_t;

/** Joystick trackball motion event structure */
typedef struct _gf_joyballevent 
{
	uint8_t type;	/**< GF_JOYBALLMOTION */
	uint8_t which;	/**< The joystick device index */
	uint8_t ball;	/**< The joystick trackball index */
	int16_t xrel;	/**< The relative motion in the X direction */
	int16_t yrel;	/**< The relative motion in the Y direction */
} gf_joyballevent_t;

/** Joystick hat position change event structure */
typedef struct _gf_joyhatevent 
{
	uint8_t type;	/**< GF_JOYHATMOTION */
	uint8_t which;	/**< The joystick device index */
	uint8_t hat;	/**< The joystick hat index */
	uint8_t value;	/**< The hat position value:
			 *   GF_HAT_LEFTUP   GF_HAT_UP       GF_HAT_RIGHTUP
			 *   GF_HAT_LEFT     GF_HAT_CENTERED GF_HAT_RIGHT
			 *   GF_HAT_LEFTDOWN GF_HAT_DOWN     GF_HAT_RIGHTDOWN
			 *  Note that zero means the POV is centered.
			 */
} gf_joyhatevent_t;

/** Joystick button event structure */
typedef struct _gf_joybuttonevent
{
	uint8_t type;	/**< GF_JOYBUTTONDOWN or GF_JOYBUTTONUP */
	uint8_t which;	/**< The joystick device index */
	uint8_t button;	/**< The joystick button index */
	uint8_t state;	/**< GF_PRESSED or GF_RELEASED */
} gf_joybuttonevent_t;

typedef struct _gf_networkevent
{
	uint8_t type; /**GF_NETWORK_EVENT*/
	uint8_t which;/**NETWORK_CABLE,NETWORK_WIFI*/
	uint8_t	mode; /**NETWORK_DHCP,NETWORK_PPPOE,NETWORK_STATIC*/
	uint8_t state;/* CONNECT_SUCCESS,CONNECT_FAILED,TIMEOUT,CABLE_ON,CABLE_OFF,IP_CONFLICT*/
	int		reason;/*AUTHOR FAILED,DISCOVERY TIMEOUT,REQUEST_TIMEOUT,NAK*/
}gf_networkevent_t;

typedef struct _gf_hardwareevent
{
	uint8_t type;
	uint8_t which; /*HDD,USB Store,Smartcard*/
	uint8_t state; /*On,Off,Full,Error,Format*/
	uint8_t wparam;
	void 	*devnode;	/* for usb,smartcard */
	void	*mountdir;
	void	*data;		/* extern parameter */
}gf_hardwareevent_t;


typedef struct _gf_systemevent
{
	uint8_t type;
	uint8_t action;/*REBOOT,SLEEP,WAKEUP,UPGRADE,LOADING*/
	uint8_t percent;
    int     upg_type;
    int     upg_status;
    int     upg_error;
}gf_systemevent_t;

typedef struct _gf_mediaevent
{
	uint8_t type;
	uint8_t action; /*BEGIN,FFWD,FBWD,PAUSE,PLAY,STOP*/
	int wparam;	
	int lparam;
	int lextend;
	void *handle;
}gf_mediaevent_t;

typedef struct _gf_browserevent
{
	uint8_t type;
	uint8_t	action; /*OPENURL,SEND A MSG,HEATBEAT*/	
	int 	code;
	int 	para;
	char*   data;  
	void *lextend;
}gf_browserevent_t;

typedef struct _gf_ntpevent
{
	uint8_t type;
	uint8_t	state; /*NTP_SYNC_SUCCESS,NTP_SYNC_FAILED*/	
}gf_ntpevent_t;

/** A user-defined event type */
typedef struct _gf_cwmpevent
{
	uint8_t type;	/**< SDL_USEREVENT through SDL_NUMEVENTS-1 */
	int code;		/**< User defined event code */
	void *data;	
}gf_cwmpevent_t;

/** A user-defined event type */
typedef struct _gf_userevent
{
	uint8_t type;	/**< SDL_USEREVENT through SDL_NUMEVENTS-1 */
	int code;		/**< User defined event code */
	void *data;		/**< User defined data pointer */
} gf_userevent_t;


/** General event structure */
typedef union _gf_event 
{
	uint8_t type;
	gf_rcevent_t rc;
	gf_kbevent_t key;
	gf_mousemotionevent_t mmotion;
	gf_mousebuttonevent_t mbutton;
	gf_joyaxisevent_t jaxis;
	gf_joyballevent_t jball;
	gf_joyhatevent_t jhat;
	gf_joybuttonevent_t jbutton;
	gf_networkevent_t network;
	gf_hardwareevent_t hardware;
	gf_systemevent_t system;
	gf_mediaevent_t media;
	gf_browserevent_t browser;
	gf_ntpevent_t ntp;
	gf_cwmpevent_t cwmp;
	gf_userevent_t user;
} gf_event_t;


typedef enum
{
    GF_EVT_REBOOT = 0,
    GF_EVT_UPGRADE,
    GF_EVT_SLEEP,
    GF_EVT_WAKEUP,
    GF_EVT_LOADING,
    GF_EVT_MAX
}e_system_event_t;

typedef enum 
{
	GF_EVT_HARDWARE_NULL = 0,
	GF_EVT_HARDWARE_HDMI,
	GF_EVT_HARDWARE_HDD,
	GF_EVT_HARDWARE_USB,
	GF_EVT_HARDWARE_USB_KEYBOARD,
	GF_EVT_HARDWARE_USB_MOUSE,
	GF_EVT_HARDWARE_MAX
}e_hardware_event_t;

typedef enum
{
	GF_BROWSER_NULL = 0,
	GF_BROWSER_OPEN_URL,
	GF_BROWSER_SEND_MSG
}gf_browseraction_t;

typedef enum
{
	GF_NTP_SYNC_SUCCESS = 0,
	GF_NTP_SYNC_FAILED,
	GF_NTP_TIMEZONE_CHANGE
}gf_ntpstate_t;


/*定义事件提交的函数类型*/
typedef int (*event_post_func)(HANDLE handle,gf_event_t* event);


#ifdef __cplusplus
}
#endif

#endif //__GFEVENT_H__

