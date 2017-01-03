#include <common.h>
//定义ipmsg协议包结构体

void msg_fun()  //消息接收判别线程 需要与其他线程通信   
{
	ipmsg_recv();   
	//识别消息类型后分类处理   此函数最为复杂，需要识别全面   例如收到用户下线广播包后从链表中删除该用户
}

void file_fun()  //文件接收线程
{
	tcp_recv();
}

int main()
{
	ipmsg_checkin();

	pthread_t msg_pid, file_pid;	
	pthread_create(&msg_pid, NULL, msg_fun, NULL);
	pthread_create(&file_pid, NULL, file_fun, NULL);
	pthread_detach(msg_pid);
	pthread_detach(file_pid);
	
	int ret;
	
	for(;;)
	{
		ret = 0;
		ret = myscanf(&ipmsg)；   //ret = 1发消息  ret = 2发文件  ret = 3下线
		if(ret == 1)
		{
			ipmsg_send();    //用udp发送
		}
		
		if(ret == 2)
		{
			ipmsg_send();   //先用udp发送文件属性，申请对方接受
			tcp_send();		//用tcp发送文件
		}
		if(ret == 3)
		{
			ipmsg_checkout();
		}
	}
	
	return 0;
}
