#include "gfapi.h"
#include "gflog.h"
#include "gfthrd.h"
#include "gfcom.h"      
#include "gfcomhandle.h"
#include "gffifo.h"
#include "gfreportlog.h"

#define FRAME_LENGTH 5

//从队列中读com口数据线程
static HANDLE m_comdata_thrd=NULL;

//COM客户端用于从接收com口数据
static ComClient_t *m_comClient = NULL;

//整理com口数据fifo
static struct cycle_buffer *m_fifo = NULL;


static int push_com_data(unsigned char *data, int size)
{
	//GFCOMHANDLE_LOG_DEBUG( "push com data size = %d\n", size);
	if(m_fifo)
		gf_fifo_put(m_fifo, data, size);
	return 0;
}

static unsigned char get_crc(unsigned char *buf,int size)
{
	int i;
	unsigned char checksum = *buf;
	for(i = 1;i < size; i ++)
	{
		checksum ^= *(buf + i); 
	}
	return checksum;
}

static int check_com_data(unsigned char *data, unsigned int size)
{
	if(data == NULL)
		return 0;
	int i = 0;

//	GFCOMHANDLE_LOG_DEBUG( "check com data size = %d\n", size);
	//查找起始标示
	if(*data == 0x10 && *(data+1) == 0x02)
	{
		//查找结束标示
		for(i = 2; i < size; i ++)
		{
			if(*(data + i) == 0x10 && *(data + i + 1) == 0x03)
			{
#if 0
				GFCOMHANDLE_LOG_DEBUG( "crc_from_data = 0x%x, crc from calc = 0x%x\n", *(data + i + 2), get_crc(data + 2, i));
#endif
				//if(*(data + i + 2) == get_crc(*(data + 2), i))
				//{
					//返回帧长度
					return i + 3;;
				//}
				//else
				{
					//CRC检验错误
					//return 0;
				}
			}

		}
		return 0;
	}
	else 
		return 0;
}



static bool on_com_data_thrd(uint64_t wParam, uint64_t lParam)
{
	unsigned char buf[64], valid_buf[64];


	while(gf_fifo_size(m_fifo) < 12)
	{
		gf_thrd_delay(50);
	}
		
	int frame_length = gf_fifo_prepare(m_fifo, buf, gf_fifo_size(m_fifo), check_com_data);

	if(0 < frame_length)
	{
		GFCOMHANDLE_LOG_DEBUG( "data_check ok origin data[%02d]:", frame_length);	
		int i = 0;
		for(i = 0; i < frame_length; i ++)
			GFCOMHANDLE_LOG_DEBUG( "%02x ", *(buf + i)); 
		printf("\n");

		memset(buf, 0, sizeof(buf));
		gf_fifo_get(m_fifo, buf, frame_length);
	}
	else
	{
#if 0
		GFCOMHANDLE_LOG_DEBUG( "data_check fail\n");	
#endif
		gf_fifo_get(m_fifo, buf, 1);

		goto END;
	}

END:
	gf_thrd_delay(10);
	return true;
}

static int param_init(ComClient_t *comClient)
{
	comClient->uart_port = 2;
	comClient->rate = 9600;
	comClient->timeout = 600*1000;
	comClient->debug  = false;
	comClient->pComDataHandler = push_com_data;
	comClient->pComTimeoutHandler = NULL;
	return 0;
}

int gf_com_handle_init(void)
{
	int ret ;

	//初始化com口数据fifo
	ret = gf_fifo_init(&m_fifo, 256);
	if(ret < 0)
	{
		GFCOMHANDLE_LOG_DEBUG("%s", "fifo2 init error");
		gf_reportlog_save(SERIAL_PORT_MODULE, "%s %s", __FUNCTION__, "fifo2 init error");
		return -1;
	}

	//初始化用于接收com口数据客户端
	m_comClient = (ComClient_t *)malloc(sizeof(ComClient_t));
	if(m_comClient == NULL)
	{
		GFCOMHANDLE_LOG_DEBUG("%s", "malloc comclient error");
		gf_reportlog_save(SERIAL_PORT_MODULE, "%s %s", __FUNCTION__, "malloc comclient error");
		return -1;
	}
	param_init(m_comClient);
	gf_com_init(m_comClient);
	
    m_comdata_thrd = gf_thrd_open("on_com_data", 100, 0, 16*1024, on_com_data_thrd, NULL, 0, 0);
	if(m_comdata_thrd)
		gf_thrd_resume(m_comdata_thrd);
	else
		gf_reportlog_save(SERIAL_PORT_MODULE, "%s init fail", __FUNCTION__);

	gf_reportlog_save(SERIAL_PORT_MODULE, "%s init suc", __FUNCTION__);
	return 0;
}

int gf_com_handle_exit(void)
{
	gf_com_exit(m_comClient);
	gf_reportlog_save(SERIAL_PORT_MODULE, "%s is exit", __FUNCTION__);
    return 0;
}
