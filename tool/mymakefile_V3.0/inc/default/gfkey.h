/** 
 * @file gfkey.h
 * @brief 定义朝歌遥控器和键盘的逻辑码表
 * @author chenkai
 * @date 2010-12-10 created
 */
#ifndef __GFKEY_H__
#define __GFKEY_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
     /** @name ASCII mapped keysyms
      *  The keyboard syms have been cleverly chosen to map to ASCII
      */
	GF_KEY_UNKNOWN	= 0,
	GF_KEY_BACKSPACE	= 8,
	GF_KEY_TAB			= 9,
	GF_KEY_CLEAR		= 12,	
	GF_KEY_ENTER		= 13,
	GF_KEY_OK			= 13,
	GF_KEY_BPAUSE		= 19,
	GF_KEY_ESCAPE		= 27,
	GF_KEY_CANCEL		= 27,
	GF_KEY_SPACE		= 32,
	GF_KEY_EXCLAIM		= 33, //!
	GF_KEY_QUOTEDBL		= 34, //"
	GF_KEY_POUND		= 35, //# 
	GF_KEY_DOLLAR		= 36, //$
	GF_KEY_PERCENT		= 37, //%
	GF_KEY_AMPERSAND	= 38, //&
	GF_KEY_QUOTE		= 39, //'
	GF_KEY_LEFTPAREN	= 40, //(
	GF_KEY_RIGHTPAREN	= 41, //)
	GF_KEY_STAR		    = 42, //*
	GF_KEY_PLUS			= 43, //+
	GF_KEY_COMMA		= 44, //,
	GF_KEY_MINUS		= 45, //-
	GF_KEY_PERIOD		= 46, //.
	GF_KEY_DOT			= 46, //.
	GF_KEY_SLASH		= 47, // /
	GF_KEY_0			= 48, 
	GF_KEY_1			= 49,
	GF_KEY_2			= 50,
	GF_KEY_3			= 51,
	GF_KEY_4			= 52,
	GF_KEY_5			= 53,
	GF_KEY_6			= 54,
	GF_KEY_7			= 55,
	GF_KEY_8			= 56,
	GF_KEY_9			= 57,
	GF_KEY_COLON		= 58, //:
	GF_KEY_SEMICOLON	= 59, //;
	GF_KEY_LESS			= 60, //<
	GF_KEY_EQUALS		= 61, //=
	GF_KEY_GREATER		= 62, //>
	GF_KEY_QUESTION		= 63, //?
	GF_KEY_AT			= 64, //@
	GF_KEY_A			= 65,
	GF_KEY_B			= 66,
	GF_KEY_C 			= 67,
	GF_KEY_D			= 68,
	GF_KEY_E			= 69,
	GF_KEY_F			= 70,
	GF_KEY_G			= 71,
	GF_KEY_H			= 72,
	GF_KEY_I			= 73,
	GF_KEY_J			= 74,
	GF_KEY_K			= 75,
	GF_KEY_L			= 76,
	GF_KEY_M			= 77,
	GF_KEY_N			= 78,
	GF_KEY_O			= 79,
	GF_KEY_P			= 80,
	GF_KEY_Q			= 81,
	GF_KEY_R			= 82,
	GF_KEY_S			= 83,
	GF_KEY_T			= 84,
	GF_KEY_U			= 85,
	GF_KEY_V			= 86,
	GF_KEY_W			= 87,
	GF_KEY_X			= 88,
	GF_KEY_Y			= 89,
	GF_KEY_Z			= 90,
	GF_KEY_LEFTBRACKET	= 91, //[
	GF_KEY_BACKSLASH	= 92, //'\'
	GF_KEY_RIGHTBRACKET	= 93, //']'
	GF_KEY_CARET		= 94, // '^'
	GF_KEY_UNDERSCORE	= 95,  //'_'
	GF_KEY_BACKQUOTE	= 96,  //'`'
	GF_KEY_a			= 97,
	GF_KEY_b			= 98,
	GF_KEY_c			= 99,
	GF_KEY_d			= 100,
	GF_KEY_e			= 101,
	GF_KEY_f			= 102,
	GF_KEY_g			= 103,
	GF_KEY_h			= 104,
	GF_KEY_i			= 105,
	GF_KEY_j			= 106,
	GF_KEY_k			= 107,
	GF_KEY_l			= 108,
	GF_KEY_m			= 109,
	GF_KEY_n			= 110,
	GF_KEY_o			= 111,
	GF_KEY_p			= 112,
	GF_KEY_q			= 113,
	GF_KEY_r			= 114,
	GF_KEY_s			= 115,
	GF_KEY_t			= 116,
	GF_KEY_u			= 117,
	GF_KEY_v			= 118,
	GF_KEY_w			= 119,
	GF_KEY_x			= 120,
	GF_KEY_y			= 121,
	GF_KEY_z			= 122,
	GF_KEY_LEFTBRACE	= 123, //{
	GF_KEY_VERTICALBAR	= 124, //|
	GF_KEY_RIGHTBRACE	= 125, //}
	GF_KEY_TILDE		= 126, //~
	GF_KEY_DEL			= 127,
	/* End of ASCII mapped keysyms */

	/** @name Arrows + Home/End pad */
	GF_KEY_UP			= 273,
	GF_KEY_DOWN			= 274,
	GF_KEY_RIGHT		= 275,
	GF_KEY_LEFT			= 276,
	GF_KEY_INSERT		= 277,
	GF_KEY_LINE_HOME	= 278, /*区别于首页，这里是来到行首*/
	GF_KEY_LINE_END		= 279,
	GF_KEY_PAGEUP		= 280,
	GF_KEY_PAGEDOWN		= 281,

	/** @name Function keys */
	GF_KEY_F1			= 282,
	GF_KEY_F2			= 283,
	GF_KEY_F3			= 284,
	GF_KEY_F4			= 285,
	GF_KEY_F5			= 286,
	GF_KEY_F6			= 287,
	GF_KEY_F7			= 288,
	GF_KEY_F8			= 289,
	GF_KEY_F9			= 290,
	GF_KEY_F10			= 291,
	GF_KEY_F11			= 292,
	GF_KEY_F12			= 293,
	GF_KEY_F13			= 294,
	GF_KEY_F14			= 295,
	GF_KEY_F15			= 296,

	/** @name Key state modifier keys */
	GF_KEY_NUMLOCK		= 300,
	GF_KEY_CAPSLOCK		= 301,
	GF_KEY_SCROLLOCK	= 302,
	GF_KEY_RSHIFT		= 303,
	GF_KEY_LSHIFT		= 304,
	GF_KEY_RCTRL		= 305,
	GF_KEY_LCTRL		= 306,
	GF_KEY_RALT			= 307,
	GF_KEY_LALT			= 308,
	GF_KEY_RMETA		= 309,
	GF_KEY_LMETA		= 310,
	GF_KEY_LSUPER		= 311,		/**< Left "Windows" key */
	GF_KEY_RSUPER		= 312,		/**< Right "Windows" key */
	GF_KEY_MODE			= 313,		/**< "Alt Gr" key */
	GF_KEY_COMPOSE		= 314,		/**< Multi-key compose key */

	/** @name Miscellaneous function keys */
	GF_KEY_HELP			= 315,
	GF_KEY_PRINT		= 316,
	GF_KEY_SYSREQ		= 317,
	GF_KEY_BREAK		= 318,
	GF_KEY_MENU			= 319,
	GF_KEY_POWER		= 320,		/**< Power Macintosh power key */
	GF_KEY_EURO			= 321,		/**< Some european keyboards */
	GF_KEY_UNDO			= 322,		/**< Atari keyboard has Undo */
	
	GF_KEY_BACK		 	= 400,	
	GF_KEY_CHANNEL_INC	= 401,
	GF_KEY_CHANNEL_DEC  = 402,
	GF_KEY_VOLUME_INC	= 403,
	GF_KEY_VOLUME_DEC	= 404,
	GF_KEY_MUTE			= 405,
	GF_KEY_AUDIO_CHAN	= 406,  /*声道*/
	GF_KEY_AUDIO_TRACK	= 407,	/*音轨*/
	GF_KEY_SUBTITLE		= 408,
	GF_KEY_PAUSE		= 409,
	GF_KEY_PLAY			= 410,
	GF_KEY_PAUSE_PLAY	= 411,
	GF_KEY_FFWD			= 412,
	GF_KEY_FBWD			= 413,
	GF_KEY_GO_END		= 414,
	GF_KEY_GO_START		= 415,
	GF_KEY_INFO			= 416,
	GF_KEY_INTERX		= 417,  //互动键
	GF_KEY_STOP			= 418,
	GF_KEY_EXIT			= 419, //退出
	GF_KEY_LAST_CHAN	= 420, //上一频道
	GF_KEY_SEEK			= 421,
	GF_KEY_HOME			= 422, //主页地址
	GF_KEY_RED			= 423,
	GF_KEY_GREEN		= 424,
	GF_KEY_YELLOW		= 425,
	GF_KEY_BLUE			= 426,
	GF_KEY_SEARCH		= 427,	
	GF_KEY_LIVETV		= 429,
	GF_KEY_VOD			= 430,
	GF_KEY_TVOD			= 431,
	GF_KEY_NVOD			= 432,
	GF_KEY_SET			= 433,
	GF_KEY_GFITCH		= 434,
	GF_KEY_FAVORITE		= 435,
	GF_KEY_DIGIT		= 436,//-/--，频道位数切换
	GF_KEY_RECORD		= 437,
	GF_KEY_IME			= 438,
	GF_KEY_COMM			= 439,
	/* Add any other keys here */
	GF_KEY_MAX
} gf_key_t;

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
} gf_keymod_t;

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
}gf_rcmodel_t;


#ifdef __cplusplus
}
#endif

#endif //__GFKEY_H__

