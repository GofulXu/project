#ifndef __SOUND_H__
#define __SOUND_H__
/*
* 语音听写(iFly Auto Transform)技术能够实时地将语音转换成对应的文字。
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"
#include "speech_recognizer.h"
#include "cJSON.h"

#define FRAME_LEN	640 
#define	BUFFER_SIZE	4096
/*
	* sub:				请求业务类型
	* domain:			领域
	* language:			语言
	* accent:			方言
	* sample_rate:		音频采样率
	* result_type:		识别结果格式
	* result_encoding:	结果编码格式
	*
	* rdn:           合成音频数字发音方式
	* volume:        合成音频的音量
	* pitch:         合成音频的音调
	* speed:         合成音频对应的语速
	* voice_name:    合成发音人
	* sample_rate:   合成音频采样率
	* text_encoding: 合成文本编码格式
	* 详细参数说明请参阅《讯飞语音云MSC--API文档》
	*/
//文字转语音参数	
#define SESSION_BEGIN_PARAMS_WRITE  "voice_name = xiaoyan, text_encoding = utf8, sample_rate = 16000, speed = 50, volume = 50, pitch = 50, rdn = 2"
//智能应答参数	----此参数有问题
#define SESSION_BEGIN_PARAMS_CHAT "sub=iat,ssm=1,sch=1,auf=audio/L16;rate=16000,aue=speex,ent=sms16k,ptt=0,rst=json,rse=utf8,nlp_version=2.0"
//语义识别+智能应答参数	
#define SESSION_BEGIN_PARAMS_ASW "sub=iat,auf=audio/L16;rate=16000,aue=speex-wb,ent=sms16k,sch=1,rst=json,rse=utf8,nlp_version=2.0"
//用户登陆参数
#define LOGIN_PARAMS  "appid = 592ae01b, work_dir = ."

//doit解析分辨类型

#define DOIT_VOICE 	1
#define DOIT_TESTCHAT	2
#define DOIT_TESTBAKE	3
#define DOIT_TESTIAAT	4
extern char *g_result;
extern unsigned int g_buffersize;

int upload_userwords();

/*****************************
*******语音识别初始化
*************************/
int sound_init(const char* login_params);


/****************
***语义识别----智能对答-语义识别
session_begin_params = 语义识别对应配置	session_begin_params = 智能对答对应配置
****************/
int voice_Smart_answer(const char *session_begin_params, const char *session_begin_params_write,const short doit_cond);
/****************
***文字识别----语义识别---智能对答
session_begin_params = 语义识别对应配置		session_begin_params = 智能对答对应配置
****************/
int voice_teSmart_answer(const char *test, const char *session_begin_params, const char *session_begin_params_write, short doit_cond);

int voice_again(const char *test, const char *session_begin_params_write);

int demo_file(const char* audio_file, const char* session_begin_params);
/* demo recognize the audio from microphone */
int demo_mic(const char* session_begin_params);
int text_to_speech(const char* src_text, const char* des_path, const char* params);
int run_iat(const char* audio_file, const char* session_begin_params,char *end_buf);


#endif
