#include "gfapi.h"
#include "gflog.h"
#include "gfthrd.h"
#include "gfudpclient.h"
#include "gfnetworkhandle.h"
#include "operate.h"
#include "GfParamerter.h"
#include "gfreportlog.h"
#ifdef SUPPORT_JNI
#include "SampleJNI.h"
#endif

#define CREATE_UDPRECV_TIMEOUT	60*60*1000
#define CREATE_UDPHEART_TIMEOUT	1*1000

typedef struct _networkhandle_para{
	char localip[64];
	unsigned int localport;
	char hostip[64];
	unsigned int hostport;
	PReportHandler ReportHandler;
	bool heart_exit;
	HANDLE heartThrd;
	SUdpClient *pClient;			//udpclient struct
							//JNI中env以及obj不允许在线程中调用,使用JavaVM指针方式
#ifdef SUPPORT_JNI
	JavaVM	*jVM;			//初始化时,保存JavaVM指针,在jvm中通用,保证在线程中可以使用	注意:不可以在回调设置函数中使用该变量,只能初始化地址
	jobject jobj;			//通过NewGlobalRef()初始化
							//通过JVM/jobj获取环境变量/类/方法
	JNIEnv *jenv;
	jclass jcla;
	jmethodID jcallback;
#endif
}networkhandle_para_t;

static networkhandle_para_t *m_para = NULL;

static int m_heart_tick = 0;					
static int m_recv_tick = 0;
static int m_recv_timeout = 10*60*1000;			//udp socket recv timeout to restart
static int m_heart_timeout = 1*1000;			//udp socket send hearts time


//参数初始化
static int param_init(void)
{
	char hostip[64];
	int hostport = 0;
	int udprecv_timeout = 0;
	int udpheart_timeout = 0;

	memset(hostip, 0, sizeof(hostip));

	if(!m_para)
		m_para = (networkhandle_para_t *)malloc(sizeof(networkhandle_para_t));
	memset(m_para, 0, sizeof(networkhandle_para_t));
	m_para->heart_exit = false;
	m_para->ReportHandler = NULL;
	m_para->heartThrd = NULL;
	m_para->pClient = NULL;			//udpclient struct
#ifdef SUPPORT_JNI
	m_para->jVM = NULL;
	m_para->jenv = NULL;
	m_para->jcla = NULL;
	m_para->jcallback = NULL;
	m_para->jobj = NULL;
#endif

	if(!gf_paramerter_get("udp_hostip", hostip, sizeof(hostip)))
		snprintf(m_para->hostip, sizeof(m_para->hostip), "%s", hostip);
	else
	{
		snprintf(m_para->hostip, sizeof(m_para->hostip), "%s", "192.168.0.242");
		gf_paramerter_set("udp_hostip", m_para->hostip);
	}

	hostport = gf_paramerter_get_int("udp_hostport");

	if(hostport <= 0 || hostport > 65535)
	{
		m_para->hostport = 5656;
		gf_paramerter_set_int("udp_hostport", m_para->hostport);
	}
	else
		m_para->hostport = (unsigned short)hostport;

	udprecv_timeout = gf_paramerter_get_int("udp_UdpRecvTimeout");
	if(udprecv_timeout <= 10*60*1000 || udprecv_timeout > 24*60*60*1000)
	{
		m_recv_timeout = CREATE_UDPRECV_TIMEOUT;
		gf_paramerter_set_int("udp_UdpRecvTimeout", m_recv_timeout);
	}else
		m_recv_timeout = udprecv_timeout;

	udpheart_timeout = gf_paramerter_get_int("udp_UdpHeartTimeout");
	if(udpheart_timeout <= 500 || udpheart_timeout > 60*1000)
	{
		m_heart_timeout = CREATE_UDPHEART_TIMEOUT;
		gf_paramerter_set_int("udp_UdpHeartTimeout", m_heart_timeout);
	}else
		m_heart_timeout = udpheart_timeout;

	int now = gf_thrd_get_tick();
	srand( now );
	m_para->localport = (unsigned short) (30000 + rand()%26000 );

	gf_reportlog_save(SERIAL_PORT_MODULE, "gfnetwork param init localip:%s, localport:%d, hostip:%s, hostport:%d, udprecv_timeout:%d, udpheart_timeout:%d\n", *m_para->localip == '\0' ? "INADDR_ANY" : m_para->localip, m_para->localport, m_para->hostip, m_para->hostport, m_recv_timeout, m_heart_timeout);
	return 0;
}

static unsigned char get_crc(char *data, int size)
{
	if(!data)
		return -1;
	int i = 0, crc_check = *data;
	for(i = 1; i < size; i++)
		crc_check ^= *(data+i);
	return (unsigned char)(crc_check << 24 >> 24);
}

static void printf_02hex(const char *format, char *data, int size)
{
	int i = 0;
	if(data)
	{
		if(format)
			GFNETWORKHANDLE_LOG_DEBUG( "%s: ", format);
		for(i = 0; i < size; i++)
			GFNETWORKHANDLE_LOG_DEBUG( "%02x ", data[i]);
		GFNETWORKHANDLE_LOG_DEBUG( "\n");
	}
	return ;
}

#ifdef SUPPORT_JNI
//jni回调接口
static int send_to_jni(char *buf, int size)
{
	//初始化类和回调对象
	if(m_para->jVM && !m_para->jcla && !m_para->jcallback)
	{
		//(*m_para->jVM)->AttachCurrentThread(m_para->jVM, &m_para->jenv, NULL);
		(*m_para->jVM)->AttachCurrentThread(m_para->jVM, (void **)&m_para->jenv, NULL);
		
		m_para->jcla = (*m_para->jenv)->GetObjectClass(m_para->jenv, m_para->jobj);
		
		if(!m_para->jcla)
		{
			GFNETWORKHANDLE_LOG_DEBUG( "javaClass not fount\n");
			return -1;
		}
		
		m_para->jcallback = (*m_para->jenv)->GetMethodID(m_para->jenv, m_para->jcla,"ongfnetworkhandle_Callback", "([BI)I");
		
		if(!m_para->jcallback)
		{
			GFNETWORKHANDLE_LOG_DEBUG( "method ongfnetworkhandle_Callback not found\n");
			return -2;
		}
	}

	//回调
	if(m_para->jVM && m_para->jenv && m_para->jcla && m_para->jcallback)
	{
		jbyteArray jarray = NULL;
		jarray = (*m_para->jenv)->NewByteArray(m_para->jenv, size);
		(*m_para->jenv)->SetByteArrayRegion(m_para->jenv, jarray, 0, size, buf);
		(*m_para->jenv)->CallVoidMethod(m_para->jenv, m_para->jobj, m_para->jcallback, jarray, size);
		(*m_para->jenv)->DeleteLocalRef(m_para->jenv, jarray);
	}else
		return -3;
	return 0;
}
#endif

//udp client接收处理函数
static bool networkhandle_recvproc( SOCKET socket, unsigned long ip, unsigned short port, char *buf, int size, unsigned long lParam )
{
	if(get_crc(buf + 2, size - 3) == buf[size-1] && *buf == 0x7E && *(buf + 1) == 0xE7)
	{
		if(m_para->ReportHandler)
			m_para->ReportHandler(ip, port, buf, size);
#ifdef SUPPORT_JNI
		send_to_jni(buf, size);
#endif
		GFNETWORKHANDLE_LOG_DEBUG( "\nchecksum suc size:%d\t", size);
		printf_02hex(__FUNCTION__, buf, size);
	}else
	{
		GFNETWORKHANDLE_LOG_DEBUG( "\nchecksum err %02x:%02x size:%d\t", get_crc(buf + 2, size - 3), buf[size-1], size);
		printf_02hex(__FUNCTION__, buf, size);
	}

	//gf_udpclient_send(pClient, ip, port, buf, size, 2000);
	m_recv_tick = gf_thrd_get_tick();

	return true;

}

//udp client错误处理函数
static bool networkhandle_errproc( SOCKET socket, unsigned long ip,	unsigned short port, unsigned long lParam )
{
	GFNETWORKHANDLE_LOG_DEBUG( "\nnetwork error %lx:%d\n", ip, port);
	return false;
}

//心跳包发送线程
static int networkhandle_heartbeat_send(SUdpClient *pClient, unsigned long ip, unsigned short port)
{
	char senddata[256];
	static short m_seq_num = 1;
	static short m_length = 8;
	memset(senddata, 0, sizeof(senddata));
	senddata[0] = 0x7E;
	senddata[1] = 0XE7;
	senddata[2] = m_length >> 8;
	senddata[3] = m_length << 8 >> 8;
	senddata[4] = m_seq_num >> 8;
	senddata[5] = m_seq_num << 8 >> 8;
	senddata[6] = 0x09;
	senddata[7] = 0x01;
	senddata[8] = m_para->localport >> 8;
	senddata[9] = m_para->localport << 8 >> 8;
	senddata[10] = 0;
	senddata[11] = 0;
	senddata[12] = get_crc(senddata + 2, 10);

	if(m_seq_num >= 65535)
		m_seq_num = 0;
	m_seq_num++;
	return gf_udpclient_send(pClient, ip, port, senddata, 13, 1000);
}


//心跳线程
static bool networkhandle_heartbeat_thrd(uint64_t wparam, uint64_t lparam )
{
	SUdpClient *pClient = (SUdpClient *)wparam;
	int ret = -1;
	static int count = 0;
	int i = 0;

	if(m_para->heart_exit)
	{
		m_para->heart_exit = false;
		m_para->heartThrd = NULL;
		return false;
	}

	if(gf_thrd_get_tick() - m_heart_tick > m_heart_timeout)
	{
		if(!gf_udpclient_isinit(pClient) || (gf_thrd_get_tick() - m_recv_tick) > m_recv_timeout)
		{
			gf_reportlog_save(SERIAL_PORT_MODULE, "gf udplient disconnect time:%dms timeout:%d\n", gf_thrd_get_tick()-m_recv_tick, m_recv_timeout);
			if(!pClient)
				pClient = (SUdpClient *)malloc(sizeof(SUdpClient));
			m_recv_tick = gf_thrd_get_tick();
			if(gf_udpclient_isinit(pClient))
				gf_udpclient_exit(pClient);
			gf_thrd_delay(1000);
			ret = gf_udpclient_init(m_para->pClient, inet_addr(m_para->localip), htons(m_para->localport), inet_addr(m_para->hostip), htons(m_para->hostport), networkhandle_recvproc, networkhandle_errproc, 0);
			if(ret == -2)
			{
				unsigned short lastport = m_para->localport;
				int now = gf_thrd_get_tick();
				srand( now );
				m_para->localport = (unsigned short) (30000 + rand()%26000 );
				gf_reportlog_save(SERIAL_PORT_MODULE, "gf udpclient init err localport:%d is used, change:%d\n", lastport, m_para->localport);
				goto WAIT;
			}
		}else
		{
			ret = networkhandle_heartbeat_send(pClient, pClient->hostip, pClient->hostport);
			if(ret < 0 && ret != -2)
			{
				if(gf_udpclient_isinit(pClient))
					gf_udpclient_exit(pClient);
			}else if(ret == 0 || ret == -2)
				count++;
			else
				count = 0;

			if(count > 5)
			{
				count = 0;
				if(gf_udpclient_isinit(pClient))
					gf_udpclient_exit(pClient);
			}
		}
		m_heart_tick = gf_thrd_get_tick();
	}

	gf_thrd_delay(100);
	return true;

WAIT:
	for(i = 0; i < 20; i++)
	{
		gf_thrd_delay(200);
		if(m_para->heart_exit)
		{
			m_para->heart_exit = false;
			m_para->heartThrd = NULL;
			return false;
		}
	}
	return true;
}

// *网络模块初始化
int gf_networkhandle_init(void)
{
	int ret = -1;
	if(!m_para)
		param_init();
	m_heart_tick = gf_thrd_get_tick();
	m_recv_tick = gf_thrd_get_tick();
	if(!m_para->pClient)
	{
		m_para->pClient = (SUdpClient *)malloc(sizeof(SUdpClient));
		memset(m_para->pClient, 0, sizeof(SUdpClient));
		ret = gf_udpclient_init(m_para->pClient, inet_addr(m_para->localip), htons(m_para->localport), inet_addr(m_para->hostip), htons(m_para->hostport), networkhandle_recvproc, networkhandle_errproc, 0);
		if(ret == -2)
		{
			unsigned short lastport = m_para->localport;
			int now = gf_thrd_get_tick();
			srand( now );
			m_para->localport = (unsigned short) (30000 + rand()%26000 );
			gf_reportlog_save(SERIAL_PORT_MODULE, "gf network udpclient init err localport:%d is used, change:%d\n", lastport, m_para->localport);
			gf_thrd_delay(5000);
		}
	}
	m_para->heartThrd = gf_thrd_open( "networkhandle_heartbeat_thrd", 50, 0, 256*1024, networkhandle_heartbeat_thrd, NULL, (uint64_t)m_para->pClient, 0);
	if(m_para->heartThrd)
		gf_thrd_resume(m_para->heartThrd);
	else
		return -1;

	gf_reportlog_save(SERIAL_PORT_MODULE, "network client init suc");
	return 0;
}

//*网络模块退出函数
void gf_networkhandle_exit(void)
{
	m_para->heart_exit = true;
	int i = 0;
	for(i = 0; i < 10; i++)
	{
		gf_thrd_delay(200);
		if(!m_para->heart_exit)
			break;
	}
	if(i >= 10 && m_para->heartThrd)
		gf_thrd_close(m_para->heartThrd, 1000);

	m_para->heart_exit = false;

	if(gf_udpclient_isinit(m_para->pClient))
		gf_udpclient_exit(m_para->pClient);

	if(m_para->pClient)
	{
		free(m_para->pClient);
		m_para->pClient = NULL;
	}
	if(m_para)
	{
		free(m_para);
		m_para = NULL;
	}

	gf_reportlog_save(SERIAL_PORT_MODULE, "network client is exit");

	return ;
}


// *设置接收回调函数
int gf_networkhandle_setcallback(PReportHandler pReportHandler)
{
	if(m_para)
	{
		m_para->ReportHandler = pReportHandler;
		return 0;
	}
	return -1;
}


#ifdef SUPPORT_JNI

JNIEXPORT jint JNICALL Java_SampleJNI_SampleInit
  (JNIEnv *env, jobject obj)
{
	return gf_networkhandle_init();
}

JNIEXPORT void JNICALL Java_SampleJNI_SampleExit
  (JNIEnv *env, jobject obj)
{
	gf_networkhandle_exit();
	return;
}

JNIEXPORT jint JNICALL Java_SampleJNI_SampleSetCallback
  (JNIEnv *env, jobject obj)
{
	m_para->jVM = NULL;
	m_para->jenv = NULL;
	m_para->jcla = NULL;
	m_para->jcallback = NULL;
	m_para->jobj = NULL;

	//保存JVM全局指针
	(*env)->GetJavaVM(env, &m_para->jVM);
	//保存obj全局对象
	m_para->jobj = (*env)->NewGlobalRef(env, obj);

	return 0;
}

#endif
