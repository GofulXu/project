#ifndef NETWORK_H
#define NETWORK_H

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdbool.h>
#include<pthread.h>
#include <unistd.h>

#define ROBOT_PORT 8080
#define ROBOT_IP "192.168.1.109"
#define ROBOT_DATA_LEN 1024

#define GEC_PORT 8080
#define GEC_IP "192.168.1.109"
#define GEC_DATA_LEN 1024

//创建套接字
int network_socket();
//创建 绑定
int network_server(int port, const char *ip);
//创建 链接服务器
int network_connect(int port, const char *ip);
//接受客户端链接
int network_accept(int sockfd, void *(*task_run)(void *));
//发送信息到ROBOT服务器
int send_to_ROBOT(const char *data);
//发送信息到ROBOT服务器并阻塞读取消息
int send_read_ROBOT(const char *w_data, char *r_buf);
int send_to_gec210(const char *data);

//发送信息到YEELINK服务器
void send_yeelink(const char *test_buf);
#endif//NETWORK_H
