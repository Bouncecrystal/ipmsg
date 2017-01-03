#include <common.h>

void ipmsg_checkin()   //绑定用户并广播，将其他用户信息存至全局链表中
{
	
}

void ipmsg_checkout()   //用户下线广播
{
	
}

int myscanf(ipmsg* msg)  //对屏幕输入信息做出判断，返回输入类型，若需要发送信息，则存入ipmsg结构体
{
	
	return ;
}

void ipmsg_send(ipmsg* msg)  //用udp发送ipmsg包
{
	
}

int ipmsg_recv(ipmsg* msg)  //用udp接受ipmsg包，返回收到包的长度
{
	
	return len;
}

tcp_send;
tcp_recv;