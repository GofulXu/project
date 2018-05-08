//////////////////////////////////////////////////////////////////
//
//  Copyright(C), 2013-2017, GEC Tech. Co., Ltd.
//
//  File name: GPLE/udp.c
//
//  Author: Vincent Lin (林世霖)  微信公众号：秘籍酷
//
//  Date: 2017-3
//  
//  Description: 网络处理模块
//
//  GitHub: github.com/vincent040   Bug Report: 2437231462@qq.com
//
//////////////////////////////////////////////////////////////////
#include <sys/types.h>			/* See NOTES */
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <linux/input.h>
#include <linux/in.h>
#include <linux/socket.h>


/*************************************************
				   变量和函数声明
 *************************************************/
#define UDP_MAX_PORT		5 
 
static int 		g_socket_fd[UDP_MAX_PORT];
static int 		g_udp_port[UDP_MAX_PORT];
static struct 	sockaddr_in g_local_addr[UDP_MAX_PORT];
static struct 	sockaddr_in g_dest_addr[UDP_MAX_PORT];

int udp_open(int udp_index,int local_port)
{
	int rt;
	
	/* 创建通信套接字 */
	g_socket_fd[udp_index] = socket(AF_INET,SOCK_DGRAM,0);
	
	if(g_socket_fd[udp_index] < 0)
	{
		printf("[udp_open]:create socket error!\n");
		return -1;			
	}
	
	int on=1;
	setsockopt(g_socket_fd[udp_index],SOL_SOCKET,SO_REUSEADDR,&on,sizeof on);

	/* 绑定通信ip与端口 */
	g_local_addr[udp_index].sin_family 			= AF_INET;
	g_local_addr[udp_index].sin_port 			= htons(local_port);
	g_local_addr[udp_index].sin_addr.s_addr 	= htonl(INADDR_ANY);

	rt = bind(g_socket_fd[udp_index],(struct sockaddr *)&g_local_addr[udp_index],sizeof(struct sockaddr_in));
	
	if(rt)
	{
		printf("[udp_open]:bind socket error!\n");
		return -1;			
	}	
	
	return 0;
}

int udp_send(int udp_index,const char *dest_addr,int dest_port,char *pudp_send_buf,int udp_send_length)
{
	g_dest_addr[udp_index].sin_family = AF_INET;
	g_dest_addr[udp_index].sin_port = htons(dest_port);
	g_dest_addr[udp_index].sin_addr.s_addr = inet_addr(dest_addr);		
	usleep(10240);
	return 	sendto(	g_socket_fd[udp_index], 
					pudp_send_buf, 
					udp_send_length, 
					0,
					(struct sockaddr *)&g_dest_addr[udp_index], 
					sizeof(struct sockaddr_in));	
}

int udp_recv(int udp_index,char *pudp_recv_buf,int udp_recv_length)
{
	int socket_length;
	
	return recvfrom(	g_socket_fd[udp_index], 
						pudp_recv_buf,
						udp_recv_length, 
						0,
						(struct sockaddr *)&g_dest_addr[udp_index], 
						&socket_length);	
	
}

int udp_close(int udp_index)
{
	if(g_socket_fd[udp_index])
	{
		close(g_socket_fd[udp_index]);
	}
	
	return 0;	
}