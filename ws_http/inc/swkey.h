/** 
 * @file swkey.h
 * @brief 定义朝歌遥控器和键盘的逻辑码表
 * @author chenkai
 * @date 2010-12-10 created
 */
#ifndef __SWKEY_H__
#define __SWKEY_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
     /** @name ASCII mapped keysyms
      *  The keyboard syms have been cleverly chosen to map to ASCII
      */
	SW_KEY_UNKNOWN	= 0,
	SW_KEY_BACKSPACE	= 8,
	SW_KEY_TAB			= 9,
	SW_KEY_CLEAR		= 12,	
	SW_KEY_ENTER		= 13,
	SW_KEY_OK			= 13,
	SW_KEY_BPAUSE		= 19,
	SW_KEY_ESCAPE		= 27,
	SW_KEY_CANCEL		= 27,
	SW_KEY_SPACE		= 32,
	SW_KEY_EXCLAIM		= 33, //!
	SW_KEY_QUOTEDBL		= 34, //"
	SW_KEY_POUND		= 35, //# 
	SW_KEY_DOLLAR		= 36, //$
	SW_KEY_PERCENT		= 37, //%
	SW_KEY_AMPERSAND	= 38, //&
	SW_KEY_QUOTE		= 39, //'
	SW_KEY_LEFTPAREN	= 40, //(
	SW_KEY_RIGHTPAREN	= 41, //)
	SW_KEY_STAR		    = 42, //*
	SW_KEY_PLUS			= 43, //+
	SW_KEY_COMMA		= 44, //,
	SW_KEY_MINUS		= 45, //-
	SW_KEY_PERIOD		= 46, //.
	SW_KEY_DOT			= 46, //.
	SW_KEY_SLASH		= 47, // /
	SW_KEY_0			= 48, 
	SW_KEY_1			= 49,
	SW_KEY_2			= 50,
	SW_KEY_3			= 51,
	SW_KEY_4			= 52,
	SW_KEY_5			= 53,
	SW_KEY_6			= 54,
	SW_KEY_7			= 55,
	SW_KEY_8			= 56,
	SW_KEY_9			= 57,
	SW_KEY_COLON		= 58, //:
	SW_KEY_SEMICOLON	= 59, //;
	SW_KEY_LESS			= 60, //<
	SW_KEY_EQUALS		= 61, //=
	SW_KEY_GREATER		= 62, //>
	SW_KEY_QUESTION		= 63, //?
	SW_KEY_AT			= 64, //@
	SW_KEY_A			= 65,
	SW_KEY_B			= 66,
	SW_KEY_C 			= 67,
	SW_KEY_D			= 68,
	SW_KEY_E			= 69,
	SW_KEY_F			= 70,
	SW_KEY_G			= 71,
	SW_KEY_H			= 72,
	SW_KEY_I			= 73,
	SW_KEY_J			= 74,
	SW_KEY_K			= 75,
	SW_KEY_L			= 76,
	SW_KEY_M			= 77,
	SW_KEY_N			= 78,
	SW_KEY_O			= 79,
	SW_KEY_P			= 80,
	SW_KEY_Q			= 81,
	SW_KEY_R			= 82,
	SW_KEY_S			= 83,
	SW_KEY_T			= 84,
	SW_KEY_U			= 85,
	SW_KEY_V			= 86,
	SW_KEY_W			= 87,
	SW_KEY_X			= 88,
	SW_KEY_Y			= 89,
	SW_KEY_Z			= 90,
	SW_KEY_LEFTBRACKET	= 91, //[
	SW_KEY_BACKSLASH	= 92, //'\'
	SW_KEY_RIGHTBRACKET	= 93, //']'
	SW_KEY_CARET		= 94, // '^'
	SW_KEY_UNDERSCORE	= 95,  //'_'
	SW_KEY_BACKQUOTE	= 96,  //'`'
	SW_KEY_a			= 97,
	SW_KEY_b			= 98,
	SW_KEY_c			= 99,
	SW_KEY_d			= 100,
	SW_KEY_e			= 101,
	SW_KEY_f			= 102,
	SW_KEY_g			= 103,
	SW_KEY_h			= 104,
	SW_KEY_i			= 105,
	SW_KEY_j			= 106,
	SW_KEY_k			= 107,
	SW_KEY_l			= 108,
	SW_KEY_m			= 109,
	SW_KEY_n			= 110,
	SW_KEY_o			= 111,
	SW_KEY_p			= 112,
	SW_KEY_q			= 113,
	SW_KEY_r			= 114,
	SW_KEY_s			= 115,
	SW_KEY_t			= 116,
	SW_KEY_u			= 117,
	SW_KEY_v			= 118,
	SW_KEY_w			= 119,
	SW_KEY_x			= 120,
	SW_KEY_y			= 121,
	SW_KEY_z			= 122,
	SW_KEY_LEFTBRACE	= 123, //{
	SW_KEY_VERTICALBAR	= 124, //|
	SW_KEY_RIGHTBRACE	= 125, //}
	SW_KEY_TILDE		= 126, //~
	SW_KEY_DEL			= 127,
	/* End of ASCII mapped keysyms */

	/** @name Arrows + Home/End pad */
	SW_KEY_UP			= 273,
	SW_KEY_DOWN			= 274,
	SW_KEY_RIGHT		= 275,
	SW_KEY_LEFT			= 276,
	SW_KEY_INSERT		= 277,
	SW_KEY_LINE_HOME	= 278, /*区别于首页，这里是来到行首*/
	SW_KEY_LINE_END		= 279,
	SW_KEY_PAGEUP		= 280,
	SW_KEY_PAGEDOWN		= 281,

	/** @name Function keys */
	SW_KEY_F1			= 282,
	SW_KEY_F2			= 283,
	SW_KEY_F3			= 284,
	SW_KEY_F4			= 285,
	SW_KEY_F5			= 286,
	SW_KEY_F6			= 287,
	SW_KEY_F7			= 288,
	SW_KEY_F8			= 289,
	SW_KEY_F9			= 290,
	SW_KEY_F10			= 291,
	SW_KEY_F11			= 292,
	SW_KEY_F12			= 293,
	SW_KEY_F13			= 294,
	SW_KEY_F14			= 295,
	SW_KEY_F15			= 296,

	/** @name Key state modifier keys */
	SW_KEY_NUMLOCK		= 300,
	SW_KEY_CAPSLOCK		= 301,
	SW_KEY_SCROLLOCK	= 302,
	SW_KEY_RSHIFT		= 303,
	SW_KEY_LSHIFT		= 304,
	SW_KEY_RCTRL		= 305,
	SW_KEY_LCTRL		= 306,
	SW_KEY_RALT			= 307,
	SW_KEY_LALT			= 308,
	SW_KEY_RMETA		= 309,
	SW_KEY_LMETA		= 310,
	SW_KEY_LSUPER		= 311,		/**< Left "Windows" key */
	SW_KEY_RSUPER		= 312,		/**< Right "Windows" key */
	SW_KEY_MODE			= 313,		/**< "Alt Gr" key */
	SW_KEY_COMPOSE		= 314,		/**< Multi-key compose key */

	/** @name Miscellaneous function keys */
	SW_KEY_HELP			= 315,
	SW_KEY_PRINT		= 316,
	SW_KEY_SYSREQ		= 317,
	SW_KEY_BREAK		= 318,
	SW_KEY_MENU			= 319,
	SW_KEY_POWER		= 320,		/**< Power Macintosh power key */
	SW_KEY_EURO			= 321,		/**< Some european keyboards */
	SW_KEY_UNDO			= 322,		/**< Atari keyboard has Undo */
	
	SW_KEY_BACK		 	= 400,	
	SW_KEY_CHANNEL_INC	= 401,
	SW_KEY_CHANNEL_DEC  = 402,
	SW_KEY_VOLUME_INC	= 403,
	SW_KEY_VOLUME_DEC	= 404,
	SW_KEY_MUTE			= 405,
	SW_KEY_AUDIO_CHAN	= 406,  /*声道*/
	SW_KEY_AUDIO_TRACK	= 407,	/*音轨*/
	SW_KEY_SUBTITLE		= 408,
	SW_KEY_PAUSE		= 409,
	SW_KEY_PLAY			= 410,
	SW_KEY_PAUSE_PLAY	= 411,
	SW_KEY_FFWD			= 412,
	SW_KEY_FBWD			= 413,
	SW_KEY_GO_END		= 414,
	SW_KEY_GO_START		= 415,
	SW_KEY_INFO			= 416,
	SW_KEY_INTERX		= 417,  //互动键
	SW_KEY_STOP			= 418,
	SW_KEY_EXIT			= 419, //退出
	SW_KEY_LAST_CHAN	= 420, //上一频道
	SW_KEY_SEEK			= 421,
	SW_KEY_HOME			= 422, //主页地址
	SW_KEY_RED			= 423,
	SW_KEY_GREEN		= 424,
	SW_KEY_YELLOW		= 425,
	SW_KEY_BLUE			= 426,
	SW_KEY_SEARCH		= 427,	
	SW_KEY_LIVETV		= 429,
	SW_KEY_VOD			= 430,
	SW_KEY_TVOD			= 431,
	SW_KEY_NVOD			= 432,
	SW_KEY_SET			= 433,
	SW_KEY_SWITCH		= 434,
	SW_KEY_FAVORITE		= 435,
	SW_KEY_DIGIT		= 436,//-/--，频道位数切换
	SW_KEY_RECORD		= 437,
	SW_KEY_IME			= 438,
	SW_KEY_COMM			= 439,
	/* Add any other keys here */
	SW_KEY_MAX
} sw_key_t;

/** Enumeration of valid key mods (possibly OR'd together) */
typedef enum 
{
	KMOD_NONE  = 0x0000,
	KMOD_LSHIFT= 0x0001,
	KMOD_RSHIFT= 0x0002,
	KMOD_LCTRL = 0x0040,
	KMOD_RCTRL = 0x0080,
	KMOD_LALT  = 0x0100,
	KMOD_RALT  = 0x0200,
	KMOD_LMETA = 0x0400,
	KMOD_RMETA = 0x0800,
	KMOD_NUM   = 0x1000,
	KMOD_CAPS  = 0x2000,
	KMOD_MODE  = 0x4000,
	KMOD_RESERVED = 0x8000
} sw_keymod_t;

#define KMOD_CTRL	(KMOD_LCTRL|KMOD_RCTRL)
#define KMOD_SHIFT	(KMOD_LSHIFT|KMOD_RSHIFT)
#define KMOD_ALT	(KMOD_LALT|KMOD_RALT)
#define KMOD_META	(KMOD_LMETA|KMOD_RMETA)

typedef enum
{
	RC_UNKNOW_MODEL =0,
	RC_FRONTPANEL,
	RC_HUAWEI_OLD,
	RC_HUAWEI_NEW,
	RC_CTC
}sw_rcmodel_t;


#ifdef __cplusplus
}
#endif

#endif //__SWKEY_H__

