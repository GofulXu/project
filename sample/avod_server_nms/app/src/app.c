#include "sys/time.h"
#include "sys/resource.h"
#include "gfapi.h"
#include "gftime.h"
#include "gflog.h"
#include "gfthrd.h"
#include "gfcomhandle.h"
#include "gfnetworkhandle.h"
#include "operate.h"
#include "gfshell.h"
#include "gfparamerter.h"
#include "gftime.h"
#include "gfreportlog.h"
#include "gfmodule.h"
#include "swjsonparser.h"
#include "cJSON.h"

#define DEFAULT_VERSION		"1.0.1"
#define DEFAULT_SERIALNO	"AVOD-01B3-20180913"
#define DEFAULT_MAC			"FF:FF:FF:FF:FF:FF"

#if 0
#define NETWORKCTLPATH		"."
#define DNSCTLPATH			"."
#define MACCTLPATH			"."
#else
#define NETWORKCTLPATH	"/etc/sysconfig/network-scripts"
#define DNSCTLPATH		"/etc"
#define MACCTLPATH		"/etc/udev/rules.d"
#endif

#define VERSIONCTLPATH		"."

typedef enum _net_cont_t{
	NET_ETH0 = 0,
	NET_ETH1,
	NET_ETH2,
	NET_ETH3
}net_cont_t;

typedef struct _net_value_t{
	char name[32];
	char device[32];
	char ipaddr[32];
	char gateway[32];
	char netmask[32];
}net_value_t;

static char m_net_list[][32] =
{
    {"NAME"},
    {"DEVICE"},
    {"IPADDR"},
    {"GATEWAY"},
	{"NETMASK"}
};

//检测URL是否合法*239.9.9.9
static bool parse_url(char *url)
{
	int i = 0;
	unsigned int buf[4];
    int ret = sscanf(url, "%d.%d.%d.%d", buf, buf+1, buf+2, buf+3);
	if(ret == 4)
	{
		for(i = 0; i < 4; i ++)
		{
			if(buf[i] > 255 || buf[i] < 0)
				return false;
		}	
		
		return true;
	}
	return false;
}

//刷新网络配置
int reset_network_config(net_cont_t cont, net_value_t *net)
{
	char netfile[128];
	char netfile1[128];
	char buf[128];
	char buf1[128];
	char buf2[128];
	int ret = 0, i = 0;
	bool name_res = false;
	bool device_res = false;
	bool ipaddr_res = false;
	bool gateway_res = false;
	bool netmask_res = false;
	memset(netfile, 0, sizeof(netfile));
	memset(netfile1, 0, sizeof(netfile1));
	memset(buf, 0, sizeof(buf));
	memset(buf1, 0, sizeof(buf1));
	memset(buf2, 0, sizeof(buf2));
	snprintf(netfile, sizeof(netfile), "%s/ifcfg-eth%d", NETWORKCTLPATH, cont);
	snprintf(netfile1, sizeof(netfile), "%s/.ifcfg-eth%d", NETWORKCTLPATH, cont);
	FILE *fd = fopen(netfile, "r");
	FILE *fd1 = fopen(netfile1, "w");

	if(!fd || !fd1)
		return -1;

	while(fgets(buf, sizeof(buf), fd))
	{
		for(i = 0; i < sizeof(m_net_list)/32; i ++)
		{
			if(strncmp(buf, m_net_list[i], strlen(m_net_list[i])) == 0)
			{
				memset(buf1, 0, sizeof(buf1));
				memset(buf2, 0, sizeof(buf2));
				switch(i){
					case 0:
							snprintf(buf1, sizeof(buf1), "%s", net->name);
							name_res = true;
							break;
					case 1:
							snprintf(buf1, sizeof(buf1), "%s", net->device);
							device_res = true;
							break;
					case 2:
							snprintf(buf1, sizeof(buf1), "%s", net->ipaddr);
							ipaddr_res = true;
							break;
					case 3:
							snprintf(buf1, sizeof(buf1), "%s", net->gateway);
							gateway_res = true;
							break;
					case 4:
							snprintf(buf1, sizeof(buf1), "%s", net->netmask);
							netmask_res = true;
							break;
					default:
							break;
				}
				if(*buf1 != '\0')
				{
					snprintf(buf2, sizeof(buf2), "%s=%s\n", m_net_list[i], buf1);
					fputs(buf2, fd1);
				}else
					fputs(buf, fd1);
				break;
			}
		}
		if(i >= sizeof(m_net_list)/32)
			fputs(buf, fd1);
		memset(buf, 0, sizeof(buf));
	}

	if(!name_res && *net->name != '\0')
	{
		memset(buf2, 0, sizeof(buf2));
		snprintf(buf2, sizeof(buf2), "%s=%s\n", m_net_list[0], net->name);
		fputs(buf2, fd1);
	}
	if(!device_res && *net->device != '\0')
	{
		memset(buf2, 0, sizeof(buf2));
		snprintf(buf2, sizeof(buf2), "%s=%s\n", m_net_list[1], net->device);
		fputs(buf2, fd1);
	}
	if(!ipaddr_res && *net->ipaddr != '\0')
	{
		memset(buf2, 0, sizeof(buf2));
		snprintf(buf2, sizeof(buf2), "%s=%s\n", m_net_list[2], net->ipaddr);
		fputs(buf2, fd1);
	}
	if(!gateway_res && *net->gateway != '\0')
	{
		memset(buf2, 0, sizeof(buf2));
		snprintf(buf2, sizeof(buf2), "%s=%s\n", m_net_list[3], net->gateway);
		fputs(buf2, fd1);
	}
	if(!netmask_res && *net->netmask != '\0')
	{
		memset(buf2, 0, sizeof(buf2));
		snprintf(buf2, sizeof(buf2), "%s=%s\n", m_net_list[4], net->netmask);
		fputs(buf2, fd1);
	}

	fclose(fd);
	fclose(fd1);
	remove(netfile);
	rename(netfile1, netfile);
	chmod(netfile, 0755);
	return 0;
}

//刷新dns配置
int reset_dns_config(char *dns1, char *dns2)
{
	char netfile[128];
	char netfile1[128];
	char buf[128];
	char buf1[128];
	int ret = 0;
	memset(netfile, 0, sizeof(netfile));
	memset(netfile1, 0, sizeof(netfile1));
	memset(buf, 0, sizeof(buf));
	memset(buf1, 0, sizeof(buf1));
	snprintf(netfile, sizeof(netfile), "%s/resolv.conf", DNSCTLPATH);
	snprintf(netfile1, sizeof(netfile1), "%s/.resolv.conf", DNSCTLPATH);
	FILE *fd = fopen(netfile, "r");
	FILE *fd1 = fopen(netfile1, "w");

	if(!fd || !fd1)
		return -1;

	while(fgets(buf, sizeof(buf), fd))
	{
		if(!strncmp(buf, "nameserver", strlen("nameserver")))
		{
			memset(buf1, 0, sizeof(buf1));
			if(!ret && dns1 && *dns1 != '\0')
				snprintf(buf1, sizeof(buf1), "nameserver %s\n", dns1);
			else if(ret == 1 && dns2 && *dns2 != '\0')
				snprintf(buf1, sizeof(buf1), "nameserver %s\n", dns2);
			ret++;
			if(*buf1 != '\0')
				fputs(buf1, fd1);
		}else
			fputs(buf, fd1);
		memset(buf, 0, sizeof(buf));
	}

	if(!ret && dns1 && *dns1 != '\0')
	{
		memset(buf1, 0, sizeof(buf1));
		snprintf(buf1, sizeof(buf1), "nameserver %s\n", dns1);
		fputs(buf1, fd1);
		ret++;
	}
	if(ret == 1 && dns2 && *dns2 != '\0')
	{
		memset(buf1, 0, sizeof(buf1));
		snprintf(buf1, sizeof(buf1), "nameserver %s\n", dns2);
		fputs(buf1, fd1);
		ret++;
	}

	fclose(fd);
	fclose(fd1);
	remove(netfile);
	rename(netfile1, netfile);
	chmod(netfile, 0755);
	return 0;
}

//获取mac参数
int get_mac_config(char *mac, int cont)
{
	char netfile[128];
	char buf[1024];
	int ret = 0, i = 0;
	char buf1[6][64];
	char buf2[6][64];
	memset(netfile, 0, sizeof(netfile));
	memset(buf, 0, sizeof(buf));
	snprintf(netfile, sizeof(netfile), "%s/70-persistent-net.rules", MACCTLPATH);
	FILE *fd = fopen(netfile, "r");
	if(!fd)
	{
		gf_reportlog_save(OPERATE_MODULE, "%s:%d---%s fopen error\n", __FUNCTION__, __LINE__, netfile);
		return -1;
	}

	while(fgets(buf, sizeof(buf), fd))
	{
		if(!strncmp(buf, "SUBSYSTEM", strlen("SUBSYSTEM")))
		{
			memset(buf1, 0, sizeof(buf1));
			memset(buf2, 0, sizeof(buf2));

			//sscanf(buf, "%[^=]==\"%[^\"]%[^=]==%[^,]%[^=]==%[^,]%[^=]==%[^,]%[^=]==%[^,]%[^=]=%[^,]", buf1[0], buf2[0], buf1[1], buf2[1], buf1[2], buf2[2], buf1[3], buf2[3], buf1[4], buf2[4], buf1[5], buf2[5]);
			sscanf(buf, "%[^=]==\"%[^\"]%[^=]==\"%[^\"]%[^=]==\"%[^\"]%[^=]==\"%[^\"]%[^=]==\"%[^\"]%[^=]=\"%[^\"]", buf1[0], buf2[0], buf1[1], buf2[1], buf1[2], buf2[2], buf1[3], buf2[3], buf1[4], buf2[4], buf1[5], buf2[5]);

			if(i == 0 && cont == 1)
			{
				strcpy(mac, buf2[3]);
			}
			else if(i == 1 && cont == 2)
			{
				strcpy(mac, buf2[3]);
			}
			else if(i == 2 && cont == 3)
			{
				strcpy(mac, buf2[3]);
			}
			else if(i == 3 && cont == 4)
			{
				strcpy(mac, buf2[3]);
			}
			i++;
		}
		memset(buf, 0, sizeof(buf));
	}

	fclose(fd);
	return ret;
}

//获取版本配置
int get_version_config(char *version, int size)
{
	char netfile[128];
	char buf[1024];
	char name[32], value[32];
	int ret = 0;
	memset(netfile, 0, sizeof(netfile));
	memset(buf, 0, sizeof(buf));
	snprintf(netfile, sizeof(netfile), "%s/version.version", VERSIONCTLPATH);
	FILE *fd = fopen(netfile, "r");
	if(!fd)
	{
		gf_reportlog_save(OPERATE_MODULE, "%s:%d---%s fopen error\n", __FUNCTION__, __LINE__, netfile);
		return -1;
	}

	while(fgets(buf, sizeof(buf), fd))
	{
		if(!strncmp(buf, "version", strlen("version")))
		{
			memset(name, 0, sizeof(name));
			memset(value, 0, sizeof(value));
			ret = sscanf(buf, "%[^=]=%[^=]", name, value);
			if(ret < 2)
				continue;
			*(value + strlen(value) - 1) = '\0';
			snprintf(version, size, "%s", value);
		}
		memset(buf, 0, sizeof(buf));
	}

	fclose(fd);
	return ret;
}

//获取dns配置
int get_dns_config(char *dns1, int size1, char *dns2, int size2)
{
	char netfile[128];
	char buf[1024];
	int ret = 0, i = 0;
	memset(netfile, 0, sizeof(netfile));
	memset(buf, 0, sizeof(buf));
	snprintf(netfile, sizeof(netfile), "%s/resolv.conf", DNSCTLPATH);
	FILE *fd = fopen(netfile, "r");
	if(!fd)
	{
		gf_reportlog_save(OPERATE_MODULE, "%s:%d---%s fopen error\n", __FUNCTION__, __LINE__, netfile);
		return -1;
	}

	while(fgets(buf, sizeof(buf), fd))
	{
		if(!strncmp(buf, "nameserver", strlen("nameserver")))
		{
			if(ret == 0)
			{
				snprintf(dns1, size1, "%s", buf + strlen("nameserver") + 1);
				*(dns1 + strlen(dns1) - 1) = '\0';
			}
			else if(ret == 1)
			{
				snprintf(dns2, size2, "%s", buf + strlen("nameserver") + 1);
				*(dns2 + strlen(dns2) - 1) = '\0';
			}
			ret++;
		}
		memset(buf, 0, sizeof(buf));
	}

	fclose(fd);
	return ret;
}

//获取网络配置
int get_network_config(net_cont_t cont, net_value_t *net)
{
	char netfile[128];
	char buf[1024];
	int ret = 0, i = 0;
	memset(netfile, 0, sizeof(netfile));
	memset(buf, 0, sizeof(buf));
	snprintf(netfile, sizeof(netfile), "%s/ifcfg-eth%d", NETWORKCTLPATH, cont);
	FILE *fd = fopen(netfile, "r");
	if(!fd)
	{
		gf_reportlog_save(OPERATE_MODULE, "%s:%d---%s fopen error\n", __FUNCTION__, __LINE__, netfile);
		return -1;
	}

	while(fgets(buf, sizeof(buf), fd))
	{
		for(i = 0; i < sizeof(m_net_list)/32; i ++)
		{
			if(strncmp(buf, m_net_list[i], strlen(m_net_list[i])) == 0)
			{
				switch(i){
					case 0:
							snprintf(net->name, sizeof(net->name), "%s", buf + strlen(m_net_list[i])+1);
							*(net->name + strlen(net->name) - 1) = '\0';
							break;
					case 1:
							snprintf(net->device, sizeof(net->device), "%s", buf + strlen(m_net_list[i])+1);
							*(net->device + strlen(net->device) - 1) = '\0';
							break;
					case 2:
							snprintf(net->ipaddr, sizeof(net->ipaddr), "%s", buf + strlen(m_net_list[i])+1);
							*(net->ipaddr + strlen(net->ipaddr) - 1) = '\0';
							break;
					case 3:
							snprintf(net->gateway, sizeof(net->gateway), "%s", buf + strlen(m_net_list[i])+1);
							*(net->gateway + strlen(net->gateway) - 1) = '\0';
							break;
					case 4:
							snprintf(net->netmask, sizeof(net->netmask), "%s", buf + strlen(m_net_list[i])+1);
							*(net->netmask + strlen(net->netmask) - 1) = '\0';
							break;
					default:
							break;
				}
			}
		}
		memset(buf, 0, sizeof(buf));
	}
	fclose(fd);
	return 0;
}

int main(int argc, char *argv[])
{
    if(argc < 2)
        return -1;

	net_value_t netvalue;
	net_value_t netvalue1;
	net_value_t netvalue2;
	net_value_t netvalue3;
	char netdns1[32], netdns2[32];
	char mac[64];
	char version[64];
	//初始化时间相关
	gf_time_init();

	//参数化参数表
	gf_paramerter_init();

	//初始化日志记录
	gf_reportlog_init();

    if(strcmp(argv[1], "GET_PARAM") == 0)
	{
		char *p = argv[2], *q = NULL;
		char param_name[64], param_value[128];
		int sendsize = 0;
		char sendbuf[2048];
		memset(sendbuf, 0, sizeof(sendbuf));
		//gf_reportlog_save(OPERATE_MODULE, "GET_PARAM,recv_data:%s\n", argv[2]);

		sendsize += snprintf(sendbuf + sendsize, sizeof(sendbuf) - sendsize, "%s", "{");

		memset(&netvalue, 0, sizeof(net_value_t));
		memset(&netvalue1, 0, sizeof(net_value_t));
		memset(&netvalue2, 0, sizeof(net_value_t));
		memset(&netvalue3, 0, sizeof(net_value_t));
		get_network_config(NET_ETH0, &netvalue);
		get_network_config(NET_ETH1, &netvalue1);
		get_network_config(NET_ETH2, &netvalue2);
		get_network_config(NET_ETH3, &netvalue3);
		memset(netdns1, 0, sizeof(netdns1));
		memset(netdns2, 0, sizeof(netdns2));
		get_dns_config(netdns1, sizeof(netdns1), netdns2, sizeof(netdns2));

		while(p)
		{
			q = strchr(p, '|');
			memset(param_name, 0, sizeof(param_name));
			if(q)
			{
				if(q - p + 1 < sizeof(param_name))
					snprintf(param_name, q - p + 1, "%s", p);
				else
				{
					p = q + 1;
					gf_reportlog_save(OPERATE_MODULE, "%s", "GET_PARAM:found too long param, read next");
					continue;
				}
			}
			else
				snprintf(param_name, sizeof(param_name), "%s", p);

			if(q)
				p = q + 1;
			else
				p = q;

			memset(param_value, 0, sizeof(param_value));

			if(strcmp(param_name, "lan_ip") == 0)
			{
				snprintf(param_value, sizeof(param_value), "%s", netvalue2.ipaddr);
			}
			else if(strcmp(param_name, "mac") == 0)
			{
				memset(mac, 0, sizeof(mac));
				get_mac_config(mac, 3);
				if(*mac != '\0')
					snprintf(param_value, sizeof(param_value), "%s", mac);
				else
					snprintf(param_value, sizeof(param_value), "%s", DEFAULT_MAC);
			}
			else if(strcmp(param_name, "serialno") == 0)
			{
				snprintf(param_value, sizeof(param_value), "%s", DEFAULT_SERIALNO);
			}
			else if(strcmp(param_name, "device_type") == 0)
			{
				snprintf(param_value, sizeof(param_value), "%s", "MDU");
			}
			else if(strcmp(param_name, "ro.build.version") == 0)
			{
				memset(version, 0, sizeof(version));
				get_version_config(version, sizeof(version));
				if(*version != '\0')
					snprintf(param_value, sizeof(param_value), "%s", version);
				else
					snprintf(param_value, sizeof(param_value), "%s", DEFAULT_VERSION);
			}
			else if(strcmp(param_name, "hardware_version") == 0)
			{
				snprintf(param_value, sizeof(param_value), "%s", "SBoxAVODSERVER");
			}
			else if(strcmp(param_name, "disk_status") == 0)
			{
				snprintf(param_value, sizeof(param_value), "%s", "7.9%");
			}
			else if(strcmp(param_name, "sleep_status") == 0)
			{
				snprintf(param_value, sizeof(param_value), "%s", "0");
			}
			else if(strcmp(param_name, "ipconflict_status") == 0)
			{
				snprintf(param_value, sizeof(param_value), "%s", "0");
			}
			else if(strncmp(param_name, "net", strlen("net")) == 0)
			{
				if(strncmp(param_name, "net_eth0", strlen("net_eth0")) == 0)
				{
					if(strncmp(param_name, "net_eth0_ipaddr", strlen("net_eth0_ipaddr")) == 0)
						snprintf(param_value, sizeof(param_value), "%s", netvalue.ipaddr);
					else if(strncmp(param_name, "net_eth0_gateway", strlen("net_eth0_gateway")) == 0)
						snprintf(param_value, sizeof(param_value), "%s", netvalue.gateway);
					else if(strncmp(param_name, "net_eth0_netmask", strlen("net_eth0_netmask")) == 0)
						snprintf(param_value, sizeof(param_value), "%s", netvalue.netmask);
				}
				else if(strncmp(param_name, "net_eth1", strlen("net_eth1")) == 0)
				{
					if(strncmp(param_name, "net_eth1_ipaddr", strlen("net_eth1_ipaddr")) == 0)
						snprintf(param_value, sizeof(param_value), "%s", netvalue1.ipaddr);
					else if(strncmp(param_name, "net_eth1_gateway", strlen("net_eth1_gateway")) == 0)
						snprintf(param_value, sizeof(param_value), "%s", netvalue1.gateway);
					else if(strncmp(param_name, "net_eth1_netmask", strlen("net_eth1_netmask")) == 0)
						snprintf(param_value, sizeof(param_value), "%s", netvalue1.netmask);
				}
				else if(strncmp(param_name, "net_eth2", strlen("net_eth2")) == 0)
				{
					if(strncmp(param_name, "net_eth2_ipaddr", strlen("net_eth2_ipaddr")) == 0)
						snprintf(param_value, sizeof(param_value), "%s", netvalue2.ipaddr);
					else if(strncmp(param_name, "net_eth2_gateway", strlen("net_eth2_gateway")) == 0)
						snprintf(param_value, sizeof(param_value), "%s", netvalue2.gateway);
					else if(strncmp(param_name, "net_eth2_netmask", strlen("net_eth2_netmask")) == 0)
						snprintf(param_value, sizeof(param_value), "%s", netvalue2.netmask);
				}
				else if(strncmp(param_name, "net_eth3", strlen("net_eth3")) == 0)
				{
					if(strncmp(param_name, "net_eth3_ipaddr", strlen("net_eth3_ipaddr")) == 0)
						snprintf(param_value, sizeof(param_value), "%s", netvalue3.ipaddr);
					else if(strncmp(param_name, "net_eth3_gateway", strlen("net_eth3_gateway")) == 0)
						snprintf(param_value, sizeof(param_value), "%s", netvalue3.gateway);
					else if(strncmp(param_name, "net_eth3_netmask", strlen("net_eth3_netmask")) == 0)
						snprintf(param_value, sizeof(param_value), "%s", netvalue3.netmask);
				}
				else if(strncmp(param_name, "net_dns1", strlen("net_dns1")) == 0)
				{
					snprintf(param_value, sizeof(param_value), "%s", netdns1);
				}
				else if(strncmp(param_name, "net_dns2", strlen("net_dns2")) == 0)
				{
					snprintf(param_value, sizeof(param_value), "%s", netdns2);
				}

			}
			else
			{
				int rtn = gf_paramerter_get(param_name, param_value, sizeof(param_value));
				if(0 > rtn)
					gf_reportlog_save(OPERATE_MODULE, "obtain param %s from database table,  errorcode = %d\n", param_name, rtn);
			}
			sendsize += snprintf(sendbuf + sendsize, sizeof(sendbuf) - sendsize, "\"%s\":\"%s\",", param_name, param_value);
		}

		sendsize --;
		sendsize += snprintf(sendbuf + sendsize, sizeof(sendbuf) - sendsize, "%s", "}");

		printf("%s", sendbuf);

		//gf_reportlog_save(OPERATE_MODULE, "GET_PARAM, send_data:%s\n", sendbuf);
	}
    else if(strcmp(argv[1], "SET_PARAM") == 0)
	{
		char value[256];
		char cmd[256];
		bool net_restart = false;
		bool net_restart1 = false;
		bool net_restart2 = false;
		bool net_restart3 = false;
		bool dns_restart = false;

		cJSON *head = cJSON_Parse(argv[2]);
		if(head)
		{
			gf_reportlog_save(OPERATE_MODULE, "cJSON Parse ok SET_PARAM:%s", argv[2]);

			memset(&netvalue, 0, sizeof(net_value_t));
			memset(&netvalue1, 0, sizeof(net_value_t));
			memset(&netvalue2, 0, sizeof(net_value_t));
			memset(&netvalue3, 0, sizeof(net_value_t));
			get_network_config(NET_ETH0, &netvalue);
			get_network_config(NET_ETH1, &netvalue1);
			get_network_config(NET_ETH2, &netvalue2);
			get_network_config(NET_ETH3, &netvalue3);
			memset(netdns1, 0, sizeof(netdns1));
			memset(netdns2, 0, sizeof(netdns2));
			get_dns_config(netdns1, sizeof(netdns1), netdns2, sizeof(netdns2));

			if(cJSON_GetObjectItem(head, "net_eth0_ipaddr"))
			{
				memset(value, 0, sizeof(value));
				snprintf(value, sizeof(value), "%s", cJSON_GetObjectItem(head, "net_eth0_ipaddr")->valuestring);

				if(strcmp(netvalue.ipaddr, value) && parse_url(value))
				{
					memset(netvalue.ipaddr, 0, sizeof(netvalue.ipaddr));
					snprintf(netvalue.ipaddr, sizeof(netvalue.ipaddr), "%s", value);
					if(0 > gf_paramerter_set("net_eth0_ipaddr", value))
						gf_paramerter_insert("net_eth0_ipaddr", value);
					net_restart = true;
				}
			}

			if(cJSON_GetObjectItem(head, "net_eth0_gateway"))
			{
				memset(value, 0, sizeof(value));
				snprintf(value, sizeof(value), "%s", cJSON_GetObjectItem(head, "net_eth0_gateway")->valuestring);

				if(strcmp(netvalue.gateway, value) && parse_url(value))
				{
					memset(netvalue.gateway, 0, sizeof(netvalue.gateway));
					snprintf(netvalue.gateway, sizeof(netvalue.gateway), "%s", value);
					if(0 > gf_paramerter_set("net_eth0_gateway", value))
						gf_paramerter_insert("net_eth0_gateway", value);
					net_restart = true;
				}
			}

			if(cJSON_GetObjectItem(head, "net_eth0_netmask"))
			{
				memset(value, 0, sizeof(value));
				snprintf(value, sizeof(value), "%s", cJSON_GetObjectItem(head, "net_eth0_netmask")->valuestring);

				if(strcmp(netvalue.netmask, value) && parse_url(value))
				{
					memset(netvalue.netmask, 0, sizeof(netvalue.netmask));
					snprintf(netvalue.netmask, sizeof(netvalue.netmask), "%s", value);
					if(0 > gf_paramerter_set("net_eth0_netmask", value))
						gf_paramerter_insert("net_eth0_netmask", value);
					net_restart = true;
				}
			}


			if(cJSON_GetObjectItem(head, "net_eth1_ipaddr"))
			{
				memset(value, 0, sizeof(value));
				snprintf(value, sizeof(value), "%s", cJSON_GetObjectItem(head, "net_eth1_ipaddr")->valuestring);

				if(strcmp(netvalue1.ipaddr, value) && parse_url(value))
				{
					memset(netvalue1.ipaddr, 0, sizeof(netvalue1.ipaddr));
					snprintf(netvalue1.ipaddr, sizeof(netvalue1.ipaddr), "%s", value);
					if(0 > gf_paramerter_set("net_eth1_ipaddr", value))
						gf_paramerter_insert("net_eth1_ipaddr", value);
					net_restart1 = true;
				}
			}

			if(cJSON_GetObjectItem(head, "net_eth1_gateway"))
			{
				memset(value, 0, sizeof(value));
				snprintf(value, sizeof(value), "%s", cJSON_GetObjectItem(head, "net_eth1_gateway")->valuestring);

				if(strcmp(netvalue1.gateway, value) && parse_url(value))
				{
					memset(netvalue1.gateway, 0, sizeof(netvalue1.gateway));
					snprintf(netvalue1.gateway, sizeof(netvalue1.gateway), "%s", value);
					if( 0 > gf_paramerter_set("net_eth1_gateway", value))
						gf_paramerter_insert("net_eth1_gateway", value);
					net_restart1 = true;
				}
			}

			if(cJSON_GetObjectItem(head, "net_eth1_netmask"))
			{
				memset(value, 0, sizeof(value));
				snprintf(value, sizeof(value), "%s", cJSON_GetObjectItem(head, "net_eth1_netmask")->valuestring);

				if(strcmp(netvalue1.netmask, value) && parse_url(value))
				{
					memset(netvalue1.netmask, 0, sizeof(netvalue1.netmask));
					snprintf(netvalue1.netmask, sizeof(netvalue1.netmask), "%s", value);
					if(0 > gf_paramerter_set("net_eth1_netmask", value))
						gf_paramerter_insert("net_eth1_netmask", value);
					net_restart1 = true;
				}
			}

			if(cJSON_GetObjectItem(head, "net_eth2_ipaddr"))
			{
				memset(value, 0, sizeof(value));
				snprintf(value, sizeof(value), "%s", cJSON_GetObjectItem(head, "net_eth2_ipaddr")->valuestring);

				if(strcmp(netvalue2.ipaddr, value) && parse_url(value))
				{
					memset(netvalue2.ipaddr, 0, sizeof(netvalue2.ipaddr));
					snprintf(netvalue2.ipaddr, sizeof(netvalue2.ipaddr), "%s", value);
					if(0 > gf_paramerter_set("net_eth2_ipaddr", value))
						gf_paramerter_insert("net_eth2_ipaddr", value);
					net_restart2 = true;
				}
			}

			if(cJSON_GetObjectItem(head, "net_eth2_gateway"))
			{
				memset(value, 0, sizeof(value));
				snprintf(value, sizeof(value), "%s", cJSON_GetObjectItem(head, "net_eth2_gateway")->valuestring);

				if(strcmp(netvalue2.gateway, value) && parse_url(value))
				{
					memset(netvalue2.gateway, 0, sizeof(netvalue2.gateway));
					snprintf(netvalue2.gateway, sizeof(netvalue2.gateway), "%s", value);
					if(0 > gf_paramerter_set("net_eth2_gateway", value))
						gf_paramerter_insert("net_eth2_gateway", value);
					net_restart2 = true;
				}
			}

			if(cJSON_GetObjectItem(head, "net_eth2_netmask"))
			{
				memset(value, 0, sizeof(value));
				snprintf(value, sizeof(value), "%s", cJSON_GetObjectItem(head, "net_eth2_netmask")->valuestring);

				if(strcmp(netvalue2.netmask, value) && parse_url(value))
				{
					memset(netvalue2.netmask, 0, sizeof(netvalue2.netmask));
					snprintf(netvalue2.netmask, sizeof(netvalue2.netmask), "%s", value);
					if(0 > gf_paramerter_set("net_eth2_netmask", value))
						gf_paramerter_insert("net_eth2_netmask", value);
					net_restart2 = true;
				}
			}


			if(cJSON_GetObjectItem(head, "net_eth3_ipaddr"))
			{
				memset(value, 0, sizeof(value));
				snprintf(value, sizeof(value), "%s", cJSON_GetObjectItem(head, "net_eth3_ipaddr")->valuestring);

				if(strcmp(netvalue3.ipaddr, value) && parse_url(value))
				{
					memset(netvalue3.ipaddr, 0, sizeof(netvalue3.ipaddr));
					snprintf(netvalue3.ipaddr, sizeof(netvalue3.ipaddr), "%s", value);
					if(0 > gf_paramerter_set("net_eth3_ipaddr", value))
						gf_paramerter_insert("net_eth3_ipaddr", value);
					net_restart3 = true;
				}
			}

			if(cJSON_GetObjectItem(head, "net_eth3_gateway"))
			{
				memset(value, 0, sizeof(value));
				snprintf(value, sizeof(value), "%s", cJSON_GetObjectItem(head, "net_eth3_gateway")->valuestring);

				if(strcmp(netvalue3.gateway, value) && parse_url(value))
				{
					memset(netvalue3.gateway, 0, sizeof(netvalue3.gateway));
					snprintf(netvalue3.gateway, sizeof(netvalue3.gateway), "%s", value);
					if(0 > gf_paramerter_set("net_eth3_gateway", value))
						gf_paramerter_insert("net_eth3_gateway", value);
					net_restart3 = true;
				}
			}

			if(cJSON_GetObjectItem(head, "net_eth3_netmask"))
			{
				memset(value, 0, sizeof(value));
				snprintf(value, sizeof(value), "%s", cJSON_GetObjectItem(head, "net_eth3_netmask")->valuestring);

				if(strcmp(netvalue3.netmask, value) && parse_url(value))
				{
					memset(netvalue3.netmask, 0, sizeof(netvalue3.netmask));
					snprintf(netvalue3.netmask, sizeof(netvalue3.netmask), "%s", value);
					if(0 > gf_paramerter_set("net_eth3_netmask", value))
						gf_paramerter_insert("net_eth3_netmask", value);
					net_restart3 = true;
				}
			}


			if(cJSON_GetObjectItem(head, "net_dns1"))
			{
				memset(value, 0, sizeof(value));
				snprintf(value, sizeof(value), "%s", cJSON_GetObjectItem(head, "net_dns1")->valuestring);

				if(strcmp(netdns1, value) && parse_url(value))
				{
					memset(netdns1, 0, sizeof(netdns1));
					snprintf(netdns1, sizeof(netdns1), "%s", value);
					if(0 > gf_paramerter_set("net_dns1", value))
						gf_paramerter_insert("net_dns1", value);
					dns_restart = true;
				}
			}

			if(cJSON_GetObjectItem(head, "net_dns2"))
			{
				memset(value, 0, sizeof(value));
				snprintf(value, sizeof(value), "%s", cJSON_GetObjectItem(head, "net_dns2")->valuestring);

				if(strcmp(netdns2, value) && parse_url(value))
				{
					memset(netdns2, 0, sizeof(netdns2));
					snprintf(netdns2, sizeof(netdns2), "%s", value);
					if(0 > gf_paramerter_set("net_dns2", value))
						gf_paramerter_insert("net_dns2", value);
					dns_restart = true;
				}
			}

			if(net_restart)
				reset_network_config(NET_ETH0, &netvalue);
			if(net_restart1)
				reset_network_config(NET_ETH1, &netvalue1);
			if(net_restart2)
				reset_network_config(NET_ETH2, &netvalue2);
			if(net_restart3)
				reset_network_config(NET_ETH3, &netvalue3);

			if(dns_restart)
				reset_dns_config(netdns1, netdns2);
#if 0
			if(net_restart)
			{
				memset(cmd, 0, sizeof(cmd));
				snprintf(cmd, sizeof(cmd), "%s", "./networkrestart.sh&");
				system(cmd);
				gf_reportlog_save(OPERATE_MODULE, "%s", "restart network");
			}
#endif
			cJSON_Delete(head);

		}else
			gf_reportlog_save(OPERATE_MODULE, "cJSON Parseerror SET_PARAM:%s", argv[2]);
	}
    else if(strcmp(argv[1], "INIT") == 0)
	{
	}
    else if(strcmp(argv[1], "CHECK_TIME") == 0)
	{
		gf_reportlog_save(OPERATE_MODULE, " adjust time to *%s*\n", argv[2]);	

		int year, mon, day, hour, min, sec;
		sscanf(argv[2], "\"%d-%d-%d %d:%d:%d\"", &year, &mon, &day, &hour, &min, &sec);

		char cmd[128] = {0};
		snprintf(cmd, sizeof(cmd), "date -s \"%04d-%02d-%02d %02d:%02d:%02d\"", year, mon, day, hour, min, sec);
		system(cmd);
	}

	gf_reportlog_exit();
	gf_time_exit();
	return 0;
}
