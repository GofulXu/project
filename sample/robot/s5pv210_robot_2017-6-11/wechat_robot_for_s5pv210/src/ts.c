//////////////////////////////////////////////////////////////////
//
//  File name: GPLE/ts.c
//
//  Author: wenwang xu (徐文旺) 
//
//  Date: 2017-5
//  
//  Description: 触摸屏处理模块
//
//  e-mail: ghu56430@qq.com
//
//////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include "tslib.h"
#include "ts.h"
#include "wechatdef.h"
static struct tsdev *g_ts=NULL;

/* 采样结果 */
static struct ts_sample g_ts_sample;

/****************
**打开触摸屏驱动
**成功返回0， 失败-1
*/
int ts_open_ex(void)
{
	/* 打开触摸屏设备 */
	g_ts= ts_open("/dev/event0", 0);
	
	if(g_ts == NULL)
		return -1;
	 
	ts_config(g_ts);	
	
	return 0;
}

/****************
**获取触摸数据
**int *x, x坐标储存地址  int *y, y坐标储存地址   int is_wait_release  是否等待长按退出，1是，0否
**成功返回0， 失败-1
*/
int ts_xy_get(int *x, int *y, int is_wait_release)
{
	int rt = ts_read(g_ts, &g_ts_sample, 1);
	if(rt < 0)
		return -1;
	while(is_wait_release)
	{
		ts_read(g_ts,&g_ts_sample,1);
		if(g_ts_sample.pressure == 0)
			break;
	}
	
	*x = g_ts_sample.x;
	*y = g_ts_sample.y;	
	
	return 0;	
}

/****************
**判断是否无触摸
**有触摸返回1，无触摸返回0
*/
int ts_is_release(void)
{
	ts_read(g_ts, &g_ts_sample, 1);
	
	printf("pressure=%d\n", g_ts_sample.pressure);
	
	if(g_ts_sample.pressure == 0)
		return 0;
	
	return 1;	
}

/****************
**关闭触摸屏文件
*/
int ts_close_ex(void)
{
	if(g_ts)
		ts_close(g_ts);
	
	return 0;	
}

/****************
**触摸数据初始化处理
**TS_DATA *robot_ts_data， 触摸数据
*/
void ts_data_init(TS_DATA *robot_ts_data)
{
	switch(robot_ts_data->itf_now_num)
	{
		case 0: robot_ts_data->ts_button_cond = 1;break;
		case ROBOT_PICONE_COND: ts_pic_one(robot_ts_data); break;
		case ROBOT_PICTWO_COND: ts_pic_two(robot_ts_data); break;
		case ROBOT_PICTHE_COND: ts_pic_the(robot_ts_data); break;
		case ROBOT_PICFOU_COND: ts_pic_fou(robot_ts_data); break;
		case ROBOT_PICFIV_COND: ts_pic_fiv(robot_ts_data); break;
		case ROBOT_PICSIX_COND: ts_pic_six(robot_ts_data); break;
		case ROBOT_PICSER_COND: ts_pic_ser(robot_ts_data); break;
		case ROBOT_PICEIG_COND: ts_pic_eig(robot_ts_data); break;
		default:break;
	}
	
}


/**************************************
**图层一按键判断处理
**TS_DATA *robot_ts_data，触摸参数
*/
void ts_pic_one(TS_DATA *robot_ts_data)
{
	if(((x0_one_button_one < robot_ts_data->ts_x) && (x1_one_button_one > robot_ts_data->ts_x)) && ((y0_one_button_one < robot_ts_data->ts_y) && (y1_one_button_one > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_one;robot_ts_data->ts_button_cond = 1;
		printf("iiiiiiiiiiiiiiiiiiiiiii \n");
	}
	else if(((x0_one_button_two < robot_ts_data->ts_x) && (x1_one_button_two > robot_ts_data->ts_x)) && ((y0_one_button_two < robot_ts_data->ts_y) && (y1_one_button_two > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_two;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_one_button_the < robot_ts_data->ts_x) && (x1_one_button_the > robot_ts_data->ts_x)) && ((y0_one_button_the < robot_ts_data->ts_y) && (y1_one_button_the > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_the;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_one_button_fou < robot_ts_data->ts_x) && (x1_one_button_fou > robot_ts_data->ts_x)) && ((y0_one_button_fou < robot_ts_data->ts_y) && (y1_one_button_fou > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fou;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_one_button_fiv < robot_ts_data->ts_x) && (x1_one_button_fiv > robot_ts_data->ts_x)) && ((y0_one_button_fiv < robot_ts_data->ts_y) && (y1_one_button_fiv > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fiv;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_one_button_six < robot_ts_data->ts_x) && (x1_one_button_six > robot_ts_data->ts_x)) && ((y0_one_button_six < robot_ts_data->ts_y) && (y1_one_button_six > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_six;robot_ts_data->ts_button_cond = 1;
	}else
	{
		robot_ts_data->ts_button_cond = 0;robot_ts_data->itf_button_type = not_press;return;
	}
	robot_ts_data->itf_button_type = short_press;
	
}

/**************************************
**图层二按键判断处理
**TS_DATA *robot_ts_data，触摸参数
*/

void ts_pic_two(TS_DATA *robot_ts_data)
{
	if(((x0_two_button_one < robot_ts_data->ts_x) && (x1_two_button_one > robot_ts_data->ts_x)) && ((y0_two_button_one < robot_ts_data->ts_y) && (y1_two_button_one > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_one;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_two_button_two < robot_ts_data->ts_x) && (x1_two_button_two > robot_ts_data->ts_x)) && ((y0_two_button_two < robot_ts_data->ts_y) && (y1_two_button_two > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_two;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_two_button_the < robot_ts_data->ts_x) && (x1_two_button_the > robot_ts_data->ts_x)) && ((y0_two_button_the < robot_ts_data->ts_y) && (y1_two_button_the > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_the;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_two_button_fou < robot_ts_data->ts_x) && (x1_two_button_fou > robot_ts_data->ts_x)) && ((y0_two_button_fou < robot_ts_data->ts_y) && (y1_two_button_fou > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fou;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_two_button_fiv < robot_ts_data->ts_x) && (x1_two_button_fiv > robot_ts_data->ts_x)) && ((y0_two_button_fiv < robot_ts_data->ts_y) && (y1_two_button_fiv > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fiv;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_two_button_six < robot_ts_data->ts_x) && (x1_two_button_six > robot_ts_data->ts_x)) && ((y0_two_button_six < robot_ts_data->ts_y) && (y1_two_button_six > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_six;robot_ts_data->ts_button_cond = 1;
	}else
	{
		robot_ts_data->ts_button_cond = 0;robot_ts_data->itf_button_type = not_press;return;
	}
	robot_ts_data->itf_button_type = short_press;
}

/**************************************
**图层三按键判断处理
**TS_DATA *robot_ts_data，触摸参数
*/

void ts_pic_the(TS_DATA *robot_ts_data)
{
	if(((x0_the_button_one < robot_ts_data->ts_x) && (x1_the_button_one > robot_ts_data->ts_x)) && ((y0_the_button_one < robot_ts_data->ts_y) && (y1_the_button_one > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_one;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_the_button_two < robot_ts_data->ts_x) && (x1_the_button_two > robot_ts_data->ts_x)) && ((y0_the_button_two < robot_ts_data->ts_y) && (y1_the_button_two > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_two;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_the_button_the < robot_ts_data->ts_x) && (x1_the_button_the > robot_ts_data->ts_x)) && ((y0_the_button_the < robot_ts_data->ts_y) && (y1_the_button_the > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_the;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_the_button_fou < robot_ts_data->ts_x) && (x1_the_button_fou > robot_ts_data->ts_x)) && ((y0_the_button_fou < robot_ts_data->ts_y) && (y1_the_button_fou > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fou;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_the_button_fiv < robot_ts_data->ts_x) && (x1_the_button_fiv > robot_ts_data->ts_x)) && ((y0_the_button_fiv < robot_ts_data->ts_y) && (y1_the_button_fiv > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fiv;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_the_button_six < robot_ts_data->ts_x) && (x1_the_button_six > robot_ts_data->ts_x)) && ((y0_the_button_six < robot_ts_data->ts_y) && (y1_the_button_six > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_six;robot_ts_data->ts_button_cond = 1;
	}else
	{
		robot_ts_data->ts_button_cond = 0;robot_ts_data->itf_button_type = not_press;return;
	}
	robot_ts_data->itf_button_type = short_press;
}

/**************************************
**图层四按键判断处理
**TS_DATA *robot_ts_data，触摸参数
*/

void ts_pic_fou(TS_DATA *robot_ts_data)
{
	if(((x0_fou_button_one < robot_ts_data->ts_x) && (x1_fou_button_one > robot_ts_data->ts_x)) && ((y0_fou_button_one < robot_ts_data->ts_y) && (y1_fou_button_one > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_one;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_fou_button_two < robot_ts_data->ts_x) && (x1_fou_button_two > robot_ts_data->ts_x)) && ((y0_fou_button_two < robot_ts_data->ts_y) && (y1_fou_button_two > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_two;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_fou_button_the < robot_ts_data->ts_x) && (x1_fou_button_the > robot_ts_data->ts_x)) && ((y0_fou_button_the < robot_ts_data->ts_y) && (y1_fou_button_the > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_the;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_fou_button_fou < robot_ts_data->ts_x) && (x1_fou_button_fou > robot_ts_data->ts_x)) && ((y0_fou_button_fou < robot_ts_data->ts_y) && (y1_fou_button_fou > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fou;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_fou_button_fiv < robot_ts_data->ts_x) && (x1_fou_button_fiv > robot_ts_data->ts_x)) && ((y0_fou_button_fiv < robot_ts_data->ts_y) && (y1_fou_button_fiv > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fiv;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_fou_button_six < robot_ts_data->ts_x) && (x1_fou_button_six > robot_ts_data->ts_x)) && ((y0_fou_button_six < robot_ts_data->ts_y) && (y1_fou_button_six > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_six;robot_ts_data->ts_button_cond = 1;
	}else
	{
		robot_ts_data->ts_button_cond = 0;robot_ts_data->itf_button_type = not_press;return;
	}
	robot_ts_data->itf_button_type = short_press;
}

/**************************************
**图层五按键判断处理
**TS_DATA *robot_ts_data，触摸参数
*/

void ts_pic_fiv(TS_DATA *robot_ts_data)
{
	if(((x0_fiv_button_one < robot_ts_data->ts_x) && (x1_fiv_button_one > robot_ts_data->ts_x)) && ((y0_fiv_button_one < robot_ts_data->ts_y) && (y1_fiv_button_one > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_one;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_fiv_button_two < robot_ts_data->ts_x) && (x1_fiv_button_two > robot_ts_data->ts_x)) && ((y0_fiv_button_two < robot_ts_data->ts_y) && (y1_fiv_button_two > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_two;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_fiv_button_the < robot_ts_data->ts_x) && (x1_fiv_button_the > robot_ts_data->ts_x)) && ((y0_fiv_button_the < robot_ts_data->ts_y) && (y1_fiv_button_the > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_the;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_fiv_button_fou < robot_ts_data->ts_x) && (x1_fiv_button_fou > robot_ts_data->ts_x)) && ((y0_fiv_button_fou < robot_ts_data->ts_y) && (y1_fiv_button_fou > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fou;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_fiv_button_fiv < robot_ts_data->ts_x) && (x1_fiv_button_fiv > robot_ts_data->ts_x)) && ((y0_fiv_button_fiv < robot_ts_data->ts_y) && (y1_fiv_button_fiv > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fiv;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_fiv_button_six < robot_ts_data->ts_x) && (x1_fiv_button_six > robot_ts_data->ts_x)) && ((y0_fiv_button_six < robot_ts_data->ts_y) && (y1_fiv_button_six > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_six;robot_ts_data->ts_button_cond = 1;
	}else
	{
		robot_ts_data->ts_button_cond = 0;robot_ts_data->itf_button_type = not_press;return;
	}
	robot_ts_data->itf_button_type = short_press;
}

/**************************************
**图层六按键判断处理
**TS_DATA *robot_ts_data，触摸参数
*/

void ts_pic_six(TS_DATA *robot_ts_data)
{
	if(((x0_six_button_one < robot_ts_data->ts_x) && (x1_six_button_one > robot_ts_data->ts_x)) && ((y0_six_button_one < robot_ts_data->ts_y) && (y1_six_button_one > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_one;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_six_button_two < robot_ts_data->ts_x) && (x1_six_button_two > robot_ts_data->ts_x)) && ((y0_six_button_two < robot_ts_data->ts_y) && (y1_six_button_two > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_two;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_six_button_the < robot_ts_data->ts_x) && (x1_six_button_the > robot_ts_data->ts_x)) && ((y0_six_button_the < robot_ts_data->ts_y) && (y1_six_button_the > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_the;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_six_button_fou < robot_ts_data->ts_x) && (x1_six_button_fou > robot_ts_data->ts_x)) && ((y0_six_button_fou < robot_ts_data->ts_y) && (y1_six_button_fou > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fou;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_six_button_fiv < robot_ts_data->ts_x) && (x1_six_button_fiv > robot_ts_data->ts_x)) && ((y0_six_button_fiv < robot_ts_data->ts_y) && (y1_six_button_fiv > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fiv;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_six_button_six < robot_ts_data->ts_x) && (x1_six_button_six > robot_ts_data->ts_x)) && ((y0_six_button_six < robot_ts_data->ts_y) && (y1_six_button_six > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_six;robot_ts_data->ts_button_cond = 1;
	}else
	{
		robot_ts_data->ts_button_cond = 0;robot_ts_data->itf_button_type = not_press;return;
	}
	robot_ts_data->itf_button_type = short_press;
}

/**************************************
**图层七按键判断处理
**TS_DATA *robot_ts_data，触摸参数
*/

void ts_pic_ser(TS_DATA *robot_ts_data)
{
	if(((x0_ser_button_one < robot_ts_data->ts_x) && (x1_ser_button_one > robot_ts_data->ts_x)) && ((y0_ser_button_one < robot_ts_data->ts_y) && (y1_ser_button_one > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_one;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_ser_button_two < robot_ts_data->ts_x) && (x1_ser_button_two > robot_ts_data->ts_x)) && ((y0_ser_button_two < robot_ts_data->ts_y) && (y1_ser_button_two > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_two;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_ser_button_the < robot_ts_data->ts_x) && (x1_ser_button_the > robot_ts_data->ts_x)) && ((y0_ser_button_the < robot_ts_data->ts_y) && (y1_ser_button_the > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_the;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_ser_button_fou < robot_ts_data->ts_x) && (x1_ser_button_fou > robot_ts_data->ts_x)) && ((y0_ser_button_fou < robot_ts_data->ts_y) && (y1_ser_button_fou > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fou;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_ser_button_fiv < robot_ts_data->ts_x) && (x1_ser_button_fiv > robot_ts_data->ts_x)) && ((y0_ser_button_fiv < robot_ts_data->ts_y) && (y1_ser_button_fiv > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fiv;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_ser_button_six < robot_ts_data->ts_x) && (x1_ser_button_six > robot_ts_data->ts_x)) && ((y0_ser_button_six < robot_ts_data->ts_y) && (y1_ser_button_six > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_six;robot_ts_data->ts_button_cond = 1;
	}else
	{
		robot_ts_data->ts_button_cond = 0;robot_ts_data->itf_button_type = not_press;return;
	}
	robot_ts_data->itf_button_type = short_press;
}

/**************************************
**图层八按键判断处理
**TS_DATA *robot_ts_data，触摸参数
*/

void ts_pic_eig(TS_DATA *robot_ts_data)
{
	if(((x0_eig_button_one < robot_ts_data->ts_x) && (x1_eig_button_one > robot_ts_data->ts_x)) && ((y0_eig_button_one < robot_ts_data->ts_y) && (y1_eig_button_one > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_one;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_eig_button_two < robot_ts_data->ts_x) && (x1_eig_button_two > robot_ts_data->ts_x)) && ((y0_eig_button_two < robot_ts_data->ts_y) && (y1_eig_button_two > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_two;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_eig_button_the < robot_ts_data->ts_x) && (x1_eig_button_the > robot_ts_data->ts_x)) && ((y0_eig_button_the < robot_ts_data->ts_y) && (y1_eig_button_the > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_the;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_eig_button_fou < robot_ts_data->ts_x) && (x1_eig_button_fou > robot_ts_data->ts_x)) && ((y0_eig_button_fou < robot_ts_data->ts_y) && (y1_eig_button_fou > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fou;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_eig_button_fiv < robot_ts_data->ts_x) && (x1_eig_button_fiv > robot_ts_data->ts_x)) && ((y0_eig_button_fiv < robot_ts_data->ts_y) && (y1_eig_button_fiv > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_fiv;robot_ts_data->ts_button_cond = 1;
	}
	else if(((x0_eig_button_six < robot_ts_data->ts_x) && (x1_eig_button_six> robot_ts_data->ts_x)) && ((y0_eig_button_six < robot_ts_data->ts_y) && (y1_eig_button_six > robot_ts_data->ts_y)))
	{	
		robot_ts_data->itf_button_num = ts_itf_button_six;robot_ts_data->ts_button_cond = 1;
	}else
	{
		robot_ts_data->ts_button_cond = 0;robot_ts_data->itf_button_type = not_press;return;
	}
	robot_ts_data->itf_button_type = short_press;
}

