/** 
 * @file swevent.h
 * @brief  参考SDL event定义事件数据结构
 * @author chenkai
 * @date 2010-12-10 created
 * @update:yihong
 */
#ifndef __SWEVENT_H__
#define __SWEVENT_H__

#include "swtype.h"
#include "swkey.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @name General keyboard/mouse state definitions */
#define SW_RELEASED	0
#define SW_PRESSED	1

/** Event enumerations */
typedef enum 
{
	SW_NOEVENT = 0,			/**< Unused (do not remove) */
	SW_RCKEYDOWN,			/**< Remote control key down */
	SW_RCKEYUP,				/**< Remote control key up */
	SW_KBKEYDOWN,			/**< Keys pressed */
	SW_KBKEYUP,				/**< Keys released */
	SW_MOUSEMOTION,			/**< Mouse moved */
	SW_MOUSEBUTTONDOWN,		/**< Mouse button pressed */
	SW_MOUSEBUTTONUP,		/**< Mouse button released */
	SW_JOYAXISMOTION,		/**< Joystick axis motion */
	SW_JOYBALLMOTION,		/**< Joystick trackball motion */
	SW_JOYHATMOTION,		/**< Joystick hat position change */
	SW_JOYBUTTONDOWN,		/**< Joystick button pressed */
	SW_JOYBUTTONUP,			/**< Joystick button released */
	SW_SYSTEM_EVENT,		/**< System event,SLEEP,WAKEUP,REBOOT,UPGRADE,QUIT*/
	SW_NETWORK_EVENT,		/**< Network event*/
	SW_HARDWARE_EVENT,		/**< Hardware event*/
	SW_MEDIA_EVENT,			/**多媒体事件*/
	SW_BROWSER_EVENT,		/**浏览器事件*/	
	SW_NTP_EVENT,			/**ntp 同步事件*/
	SW_HEARTBEAT_EVENT,		/**10ms的心跳事件*/
	SW_CWMP_EVENT,			/**终端网管事件 */
	SW_EVENT_RESERVED3,		/**< Reserved for future use.. */
	SW_EVENT_RESERVED4,		/**< Reserved for future use.. */
	SW_EVENT_RESERVED5,		/**< Reserved for future use.. */
	SW_USEREVENT = 24,		
	/** Events SW_USEREVENT through SW_MAXEVENTS-1 are for your use */
	/** This last event is only for bounding internal arrays
	 *  It is the number of bits in the event mask datatype -- Uint32
	 */
	SW_NUMEVENTS = 32
} sw_eventtype_t;

/** @name Predefined event masks */
#define SW_EVENTMASK(X)	(1<<(X))
typedef enum 
{
	SW_RCKEYDOWNMASK = SW_EVENTMASK(SW_RCKEYDOWN),
	SW_RCKEYUPMASK	 = SW_EVENTMASK(SW_RCKEYUP),
	SW_RCKEYEVENTMASK = SW_EVENTMASK(SW_RCKEYDOWN)|
		SW_EVENTMASK(SW_RCKEYUP),
	SW_KBKEYDOWNMASK = SW_EVENTMASK(SW_KBKEYDOWN),
	SW_KBKEYUPMASK	= 	SW_EVENTMASK(SW_KBKEYUP),
	SW_KBKEYEVENTMASK = SW_EVENTMASK(SW_KBKEYDOWN)|
		SW_EVENTMASK(SW_KBKEYUP),
	SW_KEYEVENTMASK	= 	SW_EVENTMASK(SW_KBKEYDOWN)|
		SW_EVENTMASK(SW_KBKEYUP)|
		SW_EVENTMASK(SW_RCKEYDOWN)|
		SW_EVENTMASK(SW_RCKEYUP),
	SW_MOUSEMOTIONMASK	= SW_EVENTMASK(SW_MOUSEMOTION),
	SW_MOUSEBUTTONDOWNMASK	= SW_EVENTMASK(SW_MOUSEBUTTONDOWN),
	SW_MOUSEBUTTONUPMASK	= SW_EVENTMASK(SW_MOUSEBUTTONUP),
	SW_MOUSEEVENTMASK	= SW_EVENTMASK(SW_MOUSEMOTION)|
		SW_EVENTMASK(SW_MOUSEBUTTONDOWN)|
		SW_EVENTMASK(SW_MOUSEBUTTONUP),
	SW_JOYAXISMOTIONMASK	= SW_EVENTMASK(SW_JOYAXISMOTION),
	SW_JOYBALLMOTIONMASK	= SW_EVENTMASK(SW_JOYBALLMOTION),
	SW_JOYHATMOTIONMASK	= SW_EVENTMASK(SW_JOYHATMOTION),
	SW_JOYBUTTONDOWNMASK	= SW_EVENTMASK(SW_JOYBUTTONDOWN),
	SW_JOYBUTTONUPMASK	= SW_EVENTMASK(SW_JOYBUTTONUP),
	SW_JOYEVENTMASK	= SW_EVENTMASK(SW_JOYAXISMOTION)|
		SW_EVENTMASK(SW_JOYBALLMOTION)|
		SW_EVENTMASK(SW_JOYHATMOTION)|
		SW_EVENTMASK(SW_JOYBUTTONDOWN)|
		SW_EVENTMASK(SW_JOYBUTTONUP),
	SW_INPUTEVENTMASK = SW_KEYEVENTMASK|SW_MOUSEEVENTMASK|SW_JOYEVENTMASK,
	SW_SYSTEMEVENTMASK = SW_EVENTMASK(SW_SYSTEM_EVENT),
	SW_NETWORKEVENTMASK = SW_EVENTMASK(SW_NETWORK_EVENT),
	SW_HARDWAREEVENTMASK = SW_EVENTMASK(SW_HARDWARE_EVENT),
	SW_MEDIAEVENTMASK = SW_EVENTMASK(SW_MEDIA_EVENT),
	SW_BROWSEREVENTMASK = SW_EVENTMASK(SW_BROWSER_EVENT),
	SW_NTPEVENTMASK = SW_EVENTMASK(SW_NTP_EVENT),
	SW_HEARTBEATEVENTMASK = SW_EVENTMASK(SW_HEARTBEAT_EVENT),
	SW_CWMPEVENTMASK = SW_EVENTMASK(SW_CWMP_EVENT),
	SW_USEREVENTMASK = SW_EVENTMASK(SW_USEREVENT)
}sw_eventmask_t;

#define SW_ALLEVENTS		0xFFFFFFFF
/**Remotecontrol event structure*/
typedef struct _sw_rcevent
{
	uint8_t type;	 /**< SW_RCKEYDOWN or SW_RCKEYUP */
	uint8_t model;	 /**< 00 FrontPanel,FF 华为普通遥控器*/
	uint8_t state;	 /**SW_PRESSED or SW_RELEASED*/
	uint32_t  phycode; /**键盘扫描码*/ 
	sw_key_t kcode;
}sw_rcevent_t;

/** Keyboard event structure */
typedef struct _sw_kbevent 
{
	uint8_t type;	/**< SW_KBKEYDOWN or SW_KBKEYUP */
	uint8_t which;	/**< The keyboard device index */
	uint8_t state;	/**< SW_PRESSED or SW_RELEASED */
	uint8_t scancode; /**< 物理码*/ 
	sw_key_t kcode;
	sw_keymod_t kmod;
}sw_kbevent_t;

/** Mouse motion event structure */
typedef struct _sw_mousemotionevent 
{
	uint8_t type;	/**< SW_MOUSEMOTION */
	uint8_t which;	/**< The mouse device index */
	uint8_t state;	/**< The current button state */
	uint16_t x, y;	/**< The X/Y coordinates of the mouse */
	int16_t xrel;	/**< The relative motion in the X direction */
	int16_t yrel;	/**< The relative motion in the Y direction */
	int16_t wheelvalue; 
} sw_mousemotionevent_t;

/** Mouse button event structure */
typedef struct _sw_mousebuttonevent 
{
	uint8_t type;	/**< SW_MOUSEBUTTONDOWN or SW_MOUSEBUTTONUP */
	uint8_t which;	/**< The mouse device index */
	uint16_t button;	/**< The mouse button index */
	uint8_t state;	/**< SW_PRESSED or SW_RELEASED */
	uint16_t x, y;	/**< The X/Y coordinates of the mouse at press time */
} sw_mousebuttonevent_t;

/** Joystick axis motion event structure */
typedef struct _sw_joyaxisevent 
{
	uint8_t type;	/**< SW_JOYAXISMOTION */
	uint8_t which;	/**< The joystick device index */
	uint8_t axis;	/**< The joystick axis index */
	int16_t value;	/**< The axis value (range: -32768 to 32767) */
} sw_joyaxisevent_t;

/** Joystick trackball motion event structure */
typedef struct _sw_joyballevent 
{
	uint8_t type;	/**< SW_JOYBALLMOTION */
	uint8_t which;	/**< The joystick device index */
	uint8_t ball;	/**< The joystick trackball index */
	int16_t xrel;	/**< The relative motion in the X direction */
	int16_t yrel;	/**< The relative motion in the Y direction */
} sw_joyballevent_t;

/** Joystick hat position change event structure */
typedef struct _sw_joyhatevent 
{
	uint8_t type;	/**< SW_JOYHATMOTION */
	uint8_t which;	/**< The joystick device index */
	uint8_t hat;	/**< The joystick hat index */
	uint8_t value;	/**< The hat position value:
			 *   SW_HAT_LEFTUP   SW_HAT_UP       SW_HAT_RIGHTUP
			 *   SW_HAT_LEFT     SW_HAT_CENTERED SW_HAT_RIGHT
			 *   SW_HAT_LEFTDOWN SW_HAT_DOWN     SW_HAT_RIGHTDOWN
			 *  Note that zero means the POV is centered.
			 */
} sw_joyhatevent_t;

/** Joystick button event structure */
typedef struct _sw_joybuttonevent
{
	uint8_t type;	/**< SW_JOYBUTTONDOWN or SW_JOYBUTTONUP */
	uint8_t which;	/**< The joystick device index */
	uint8_t button;	/**< The joystick button index */
	uint8_t state;	/**< SW_PRESSED or SW_RELEASED */
} sw_joybuttonevent_t;

typedef struct _sw_networkevent
{
	uint8_t type; /**SW_NETWORK_EVENT*/
	uint8_t which;/**NETWORK_CABLE,NETWORK_WIFI*/
	uint8_t	mode; /**NETWORK_DHCP,NETWORK_PPPOE,NETWORK_STATIC*/
	uint8_t state;/* CONNECT_SUCCESS,CONNECT_FAILED,TIMEOUT,CABLE_ON,CABLE_OFF,IP_CONFLICT*/
	int		reason;/*AUTHOR FAILED,DISCOVERY TIMEOUT,REQUEST_TIMEOUT,NAK*/
}sw_networkevent_t;

typedef struct _sw_hardwareevent
{
	uint8_t type;
	uint8_t which; /*HDD,USB Store,Smartcard*/
	uint8_t state; /*On,Off,Full,Error,Format*/
	uint8_t wparam;
	void 	*devnode;	/* for usb,smartcard */
	void	*mountdir;
	void	*data;		/* extern parameter */
}sw_hardwareevent_t;


typedef struct _sw_systemevent
{
	uint8_t type;
	uint8_t action;/*REBOOT,SLEEP,WAKEUP,UPGRADE,LOADING*/
	uint8_t percent;
    int     upg_type;
    int     upg_status;
    int     upg_error;
}sw_systemevent_t;

typedef struct _sw_mediaevent
{
	uint8_t type;
	uint8_t action; /*BEGIN,FFWD,FBWD,PAUSE,PLAY,STOP*/
	int wparam;	
	int lparam;
	int lextend;
	void *handle;
}sw_mediaevent_t;

typedef struct _sw_browserevent
{
	uint8_t type;
	uint8_t	action; /*OPENURL,SEND A MSG,HEATBEAT*/	
	int 	code;
	int 	para;
	char*   data;  
	void *lextend;
}sw_browserevent_t;

typedef struct _sw_ntpevent
{
	uint8_t type;
	uint8_t	state; /*NTP_SYNC_SUCCESS,NTP_SYNC_FAILED*/	
}sw_ntpevent_t;

/** A user-defined event type */
typedef struct _sw_cwmpevent
{
	uint8_t type;	/**< SDL_USEREVENT through SDL_NUMEVENTS-1 */
	int code;		/**< User defined event code */
	void *data;	
}sw_cwmpevent_t;

/** A user-defined event type */
typedef struct _sw_userevent
{
	uint8_t type;	/**< SDL_USEREVENT through SDL_NUMEVENTS-1 */
	int code;		/**< User defined event code */
	void *data;		/**< User defined data pointer */
} sw_userevent_t;


/** General event structure */
typedef union _sw_event 
{
	uint8_t type;
	sw_rcevent_t rc;
	sw_kbevent_t key;
	sw_mousemotionevent_t mmotion;
	sw_mousebuttonevent_t mbutton;
	sw_joyaxisevent_t jaxis;
	sw_joyballevent_t jball;
	sw_joyhatevent_t jhat;
	sw_joybuttonevent_t jbutton;
	sw_networkevent_t network;
	sw_hardwareevent_t hardware;
	sw_systemevent_t system;
	sw_mediaevent_t media;
	sw_browserevent_t browser;
	sw_ntpevent_t ntp;
	sw_cwmpevent_t cwmp;
	sw_userevent_t user;
} sw_event_t;


typedef enum
{
    SW_EVT_REBOOT = 0,
    SW_EVT_UPGRADE,
    SW_EVT_SLEEP,
    SW_EVT_WAKEUP,
    SW_EVT_LOADING,
    SW_EVT_MAX
}e_system_event_t;

typedef enum 
{
	SW_EVT_HARDWARE_NULL = 0,
	SW_EVT_HARDWARE_HDMI,
	SW_EVT_HARDWARE_HDD,
	SW_EVT_HARDWARE_USB,
	SW_EVT_HARDWARE_USB_KEYBOARD,
	SW_EVT_HARDWARE_USB_MOUSE,
	SW_EVT_HARDWARE_MAX
}e_hardware_event_t;

typedef enum
{
	SW_BROWSER_NULL = 0,
	SW_BROWSER_OPEN_URL,
	SW_BROWSER_SEND_MSG
}sw_browseraction_t;

typedef enum
{
	SW_NTP_SYNC_SUCCESS = 0,
	SW_NTP_SYNC_FAILED,
	SW_NTP_TIMEZONE_CHANGE
}sw_ntpstate_t;


/*定义事件提交的函数类型*/
typedef int (*event_post_func)(HANDLE handle,sw_event_t* event);


#ifdef __cplusplus
}
#endif

#endif //__SWEVENT_H__

