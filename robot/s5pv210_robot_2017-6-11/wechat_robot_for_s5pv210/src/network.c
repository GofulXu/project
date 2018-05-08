#include "network.h"

/*********************
**创建套接字
**成功返回0，失败-1
*/
int network_socket()
{
	int sockfd = socket(AF_INET, SOCK_STREAM,0);
	if(sockfd < 0)
	{
		perror("socket error");
		return -1;
	}
	return sockfd;
}

/*********************
**创建 绑定
**int port,端口 const char *ip，ip地址
**成功返回0，失败-1
**注:(如果ip为NULL就用默认)
*/
int network_server(int port, const char *ip)
{
	int sockfd = network_socket();
	//地址复用
	int on = 1;
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if(ret < 0)
	{
		perror("set reuse error");
		return -1;
	}
	
	//2.绑定端口地址
	struct sockaddr_in baddr;
	bzero(&baddr, sizeof(baddr));
	baddr.sin_family = AF_INET ;
	baddr.sin_port = htons(port);
	if(ip == NULL)
	{
		baddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}else
	{
		baddr.sin_addr.s_addr = inet_addr(ip);		
	}
	
	//绑定
	ret = bind(sockfd, (struct sockaddr*)&baddr, sizeof(baddr));
	if(ret < 0)
	{
		perror("bind error");
		return -1;
	}
	//监听
	ret = listen(sockfd,5);
	if(ret < 0)
	{
			perror("listen fail");
			return -1;
	}
	return sockfd;
	
}

/*********************
**创建 链接服务器
**int port,端口 const char *ip，ip地址
**成功返回0，失败-1
*/
int network_connect(int port, const char *ip)
{
	int sockfd = network_socket();	
	//2.绑定端口地址
	struct sockaddr_in baddr;
	bzero(&baddr, sizeof(baddr));
	baddr.sin_family = AF_INET ;
	baddr.sin_port = htons(port);
	baddr.sin_addr.s_addr = inet_addr(ip);		
	
	int ret = connect(sockfd, (struct sockaddr*)&baddr, sizeof(baddr));
	if(ret < 0)
	{
		perror("connect error");
		return -1;
	}
	return sockfd;
}

/*********************
**接受客户端链接(创建服务器后使用)int network_server(int port, const char *ip)
**int sockfd, 套接字描述符  void *(*task_run)(void *) 处理线程
**成功返回0，失败-1
*/
int network_accept(int sockfd, void *(*task_run)(void *))
{
	struct sockaddr_in cltaddr;
	socklen_t len = sizeof(cltaddr);
	int clientfd = accept(sockfd,(struct sockaddr*)&cltaddr,&len);
	if(clientfd < 0)
	{
		perror("accept error");
		return -1;
	}	
	
	//鍒涘缓绾跨▼
	pthread_t id = 0;
	pthread_create(&id, NULL, task_run, (void*)&clientfd);
	pthread_detach(id);
	return 0;
}


/*********************
**发送信息到ROBOT服务器
**const char *data，发送的数据
**成功返回0，失败-1
*/
int send_to_ROBOT(const char *data)
{
	int confd = network_connect(ROBOT_PORT, ROBOT_IP);
	if(confd < 0){
		perror("connect error:");
		return -1;
	}
	write(confd, data, strlen(data));
	close(confd);
	return 0;
}

int send_to_gec210(const char *data)
{
	int confd = network_connect(GEC_PORT, GEC_IP);
	if(confd < 0){
		perror("connect error:");
		return -1;
	}
	write(confd, data, strlen(data));
	close(confd);
	return 0;
}

/*********************
**发送信息到ROBOT服务器并阻塞读取消息
**const char *w_data，发送的数据   char *r_buf， 接受数据的地址
**成功返回0，失败-1
*/
int send_read_ROBOT(const char *w_data, char *r_buf)
{
	int confd = network_connect(ROBOT_PORT, ROBOT_IP);
	if(confd < 0){
		perror("connect error:");
		return -1;
	}
	write(confd, w_data, strlen(w_data));
	read(confd, r_buf, ROBOT_DATA_LEN);
	close(confd);
	return 0;
}

/*********************
**发送传感器信息到yeelink服务器
**const char *test_buf, 发送的数据
**成功返回0，失败-1
*/
void send_yeelink(const char *test_buf)
{
	int temp = 0,humi = 0;
	char delim[] = ":";
	char str[1024] = {0};
	char *s = strdup(test_buf);
	strsep(&s, delim);
	if(strcmp(strsep(&s, delim),"temp") == 0)		//姝ｅ父msg
	{
		temp = atoi(strsep(&s, delim));
	}
	if(strcmp(strsep(&s, delim),"humi") == 0)		//姝ｅ父msg
	{
		humi = atoi(strsep(&s, delim));
	}
	printf("temp:%d----humi:%d\n",temp, humi);
	sprintf(str, "curl --request POST --data \"{\\\"value\\\":%d}\" --header \"U-ApiKey: 4f132e6649069cc4299bacdc2e4b2d02\"  http://api.yeelink.net/v1.0/device/350433/sensor/393269/datapoints",temp);
	system(str);
	memset(str,0,1024);
	sprintf(str, "curl --request POST --data \"{\\\"value\\\":%d}\" --header \"U-ApiKey: 4f132e6649069cc4299bacdc2e4b2d02\"  http://api.yeelink.net/v1.0/device/350433/sensor/402844/datapoints",humi);
	system(str);
	system("wget \"http://192.168.1.251:8080/?action=snapshot\" -O /mnt/hgfs/share/yeelink/pic/pic.jpg");
	system("wget --post-file=/mnt/hgfs/share/yeelink/pic/pic.jpg --head=\"U-ApiKey:4f132e6649069cc4299bacdc2e4b2d02\" \"http://api.yeelink.net/v1.0/device/350433/sensor/402697/photos\"");
	return;
}
