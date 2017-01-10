#include "../include/common.h"

extern struct list_head head;  //引入main.c中全局变量用户链表表头

typedef struct user_list   //定义用户链表
{
	char name[10];
	char machine[16];
	char ip[16];
	struct list_head list;
}User_list;

typedef struct ipmsg  //定义ipmsg解析结构体
{
	char name[10];
	char machine[16];
	char command[33];
	char buffer[1024];
}Ipmsg;

static void list_printf();  //用户链表遍历函数
static void help_printf();  //打印help界面
static void ipmsg_analyze(char* msg, Ipmsg* ipmsg);  //ipmsg解析函数
static void ipmsg_checkin(int sockfd, char* ip, char* user);
static void ipmsg_checkout(int sockfd, char* ip, char* user);
static void ipmsg_send(int sockfd, char* toip, char* ipmsg);

void bc_recv(int bc_sockfd,int udp_sockfd, char* myip, char* user);
void ipmsg_recv(int udp_sockfd, char* myip, char* user);
void myscanf(int bc_sockfd, int udp_sockfd, char* myip, char* user);

static void list_printf()  //用户链表遍历函数
{
	User_list* p;
	struct list_head *pos;
	list_for_each(pos, &head)
	{
		p = list_entry(pos, User_list, list);
		printf("%s %s %s\n", p->name, p->machine, p->ip);
	}
}

static void help_printf()  //help界面打印函数
{
	printf("*****************************************************************\n\n");
	printf("* send to [username]:                            :发送消息	*\n\n");
	printf("* sendfile to [username]:[filename]		 :发送文件	*\n\n");
	printf("* user 					         :打印用户列表	*\n\n");
	printf("* ls                                             :打印文件列表	*\n\n");
	printf("* clear					         :清屏	        *\n\n");
	printf("* help						 :帮助	        *\n\n");
	printf("* exit   					 :退出	        *\n\n");
	printf("*****************************************************************\n\n");
}

static void ipmsg_analyze(char* msg, Ipmsg* ipmsg)  //ipmsg解析
{
	int i, j, k;
	for(i = 0, j = 0, k = 0; i < 1024; i++)
	{
		if(j == 2)
		{
			ipmsg->name[k++] = msg[i];
			if(msg[i] == ':')
			{
				ipmsg->name[k-1] = '\0';
				j++;
				k = 0;
			}
		}
		else if(j == 3)
		{
			ipmsg->machine[k++] = msg[i];
			if(msg[i] == ':')
			{
				ipmsg->machine[k-1] = '\0';
				j++;
				k = 0;
			}
		}
		else if(j == 4)
		{
			ipmsg->command[k++] = msg[i];
			if(msg[i] == ':')
			{
				ipmsg->command[k-1] = '\0';
				j++;
				k = 0;
			}
		}
		else if(j == 5)
		{
			ipmsg->buffer[k++] = msg[i];
			if(msg[i] == ':')
			{
				ipmsg->buffer[k-1] = '\0';
				break;
			}
		}
		else if(msg[i] == ':')
			j++;
	}	
}

static void ipmsg_checkin(int sockfd, char* myip, char* user)    //用户上线广播
{	
	char ipmsg[1024];
	sprintf(ipmsg, "1:%ld:%s:%s:%ld:", time(NULL), user, myip, IPMSG_BR_ENTRY);
	printf("entry msg:%s\n", ipmsg);
	ipmsg_send(sockfd, "192.168.31.255", ipmsg);
}

static void ipmsg_checkout(int sockfd, char* myip, char* user)   //用户下线广播
{
	char ipmsg[1024];
	sprintf(ipmsg, "1:%ld:%s:%s:%ld:", time(NULL), user, myip, IPMSG_BR_EXIT);
	printf("exit msg:%s\n", ipmsg);
	ipmsg_send(sockfd, "192.168.31.255", ipmsg);
	
	close(sockfd);
}

static void ipmsg_send(int sockfd, char* toip, char* ipmsg)  //用udp发送ipmsg包
{
	int ret;
	
	struct sockaddr_in toaddr;
	toaddr.sin_family = AF_INET;
	toaddr.sin_port = htons(2325);
	inet_pton(AF_INET, toip, &toaddr.sin_addr);

	ret = sendto(sockfd, ipmsg, strlen(ipmsg), 0, (struct sockaddr*)&toaddr, sizeof(toaddr));
	if(ret < 0)
		err_sys("sendto error");
}

void bc_recv(int bc_sockfd, int udp_sockfd, char* myip, char* user)  //接收广播
{
	int ret;
			
	const int opt = 1;
	ret = setsockopt(bc_sockfd, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, (char*)&opt, sizeof(opt));
	if(ret < 0)
		err_sys("setsockopt error");
			
	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_port   = htons(2325); 
	inet_pton(AF_INET, "192.168.31.255", &myaddr.sin_addr);
	
	ret = bind(bc_sockfd, (struct sockaddr*)&myaddr, sizeof(myaddr));
	if(ret != 0)
		err_sys("bind error in ipmsg recv");
	
	ipmsg_checkin(bc_sockfd, myip, user);
	
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(2325);
	socklen_t len = sizeof(addr);
	
	int n;
	char recvipmsg[1024];
	char sendipmsg[1024];
	char ip[16];
	Ipmsg ipmsg;

	while(1)
	{
		n = 0;
		bzero(recvipmsg, sizeof(recvipmsg));
		bzero(sendipmsg, sizeof(sendipmsg));
		bzero(ip, sizeof(ip));
		bzero(&ipmsg, sizeof(ipmsg));
		
		n = recvfrom(bc_sockfd, recvipmsg, sizeof(recvipmsg), 0, (struct sockaddr*)&addr, &len);
		if(n < 0)
			err_sys("recvfrom error");
		inet_ntop(AF_INET, &addr.sin_addr, ip, 16);
	
		printf("ipmsg recv:%s\n", recvipmsg);
		
		ipmsg_analyze(recvipmsg, &ipmsg);
		
		if(atol(ipmsg.command) == IPMSG_BR_ENTRY)  //收到上线通知，回复在线确认
		{
			printf("%s entry\n", ipmsg.name);     //xx上线
			
			User_list *tmp = (User_list *)malloc(sizeof(User_list));
			ASSERT(tmp);
			int i;
			for(i = 0; i < 10; i++)
			{
				tmp->name[i] = ipmsg.name[i];
				if(ipmsg.name[i] == '\0')
					break;
			}
			for(i = 0; i < 16; i++)
			{
				tmp->machine[i] = ipmsg.machine[i];
				if(ipmsg.machine[i] == '\0')
					break;
			}
			for(i = 0; i < 16; i++)
			{
				tmp->ip[i] = ip[i];
				if(ip[i] == '\0')
					break;
			}

			list_add(&tmp->list,&head);    //添加至用户列表
			
			//if(strcmp(ipmsg.name, user) == 0) 
			//	continue;
			
			sprintf(sendipmsg, "1:%ld:%s:%s:%ld:", time(NULL), user, myip, IPMSG_ANSENTRY);
			ipmsg_send(udp_sockfd, ip, sendipmsg);
			printf("ipmsg send:%s\n", sendipmsg);
		}
		
		if(atol(ipmsg.command) == IPMSG_BR_EXIT)  //收到下线通知，删除用户
		{
			printf("%s exit\n", ipmsg.name);
			
			//删除用户

			User_list* p;
			struct list_head *pos;
			struct list_head *n;
			list_for_each_safe(pos, n, &head);
			{
				p = list_entry(pos, User_list, list);
				if(strcmp(p->name, ipmsg.name) == 0)
				{
					printf("%s\n", p->name);
					free(p);
					break;
				}
			}
		}
	}
}

void ipmsg_recv(int udp_sockfd, char* myip, char* user)  //用udp接受ipmsg包
{
	int ret;

	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(2325); 
	inet_pton(AF_INET, myip, &myaddr.sin_addr);
	
	ret = bind(udp_sockfd, (struct sockaddr*)&myaddr, sizeof(myaddr));
	if(ret != 0)
		err_sys("bind error in ipmsg recv");
	
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(2325);
	socklen_t len = sizeof(addr);
	
	int n;
	char recvipmsg[1024];
	char sendipmsg[1024];
	char ip[16];
	Ipmsg ipmsg;

	while(1)
	{
		n = 0;
		bzero(recvipmsg, sizeof(recvipmsg));
		bzero(sendipmsg, sizeof(sendipmsg));
		bzero(ip, sizeof(ip));
		bzero(&ipmsg, sizeof(ipmsg));
		
		n = recvfrom(udp_sockfd, recvipmsg, sizeof(recvipmsg), 0, (struct sockaddr*)&addr, &len);
		if(n < 0)
			err_sys("recvfrom error");
		inet_ntop(AF_INET, &addr.sin_addr, ip, 16);
	
		printf("ipmsg recv:%s\n", recvipmsg);
		
		ipmsg_analyze(recvipmsg, &ipmsg);
		
		if(atol(ipmsg.command) == IPMSG_ANSENTRY)   //收到在线确认，添加至用户链表
		{
			if(strcmp(ipmsg.name, user) != 0)
			{
				User_list *tmp = (User_list *)malloc(sizeof(User_list));
				ASSERT(tmp);
				int i;
				for(i = 0; i < 10; i++)
				{
					tmp->name[i] = ipmsg.name[i];
					if(ipmsg.name[i] == '\0')
						break;
				}
				for(i = 0; i < 16; i++)
				{
					tmp->machine[i] = ipmsg.machine[i];
					if(ipmsg.machine[i] == '\0')
					break;
				}
				for(i = 0; i < 16; i++)
				{
					tmp->ip[i] = ip[i];
					if(ip[i] == '\0')
						break;
				}
				list_add(&tmp->list,&head); 
			}
		}
		
		if(atol(ipmsg.command) == IPMSG_SENDMSG)    //收到消息
		{
			printf("recv from %s:%s\n", ipmsg.name, ipmsg.buffer);
		}
		
		//if(atoi(command) == IPMSG_RECVMSG)    //收到消息确认
		//if(atoi(command) == IPMSG_GETFILEDATA)  //收到文件发送请求
		//if(atoi(command) == IPMSG_RELEASEFILES)  //收到取消文件发送请求
		//if(atoi(command) == IPMSG_GETDIRFILES)   //请求传输文件夹
		
	}
}

void myscanf(int bc_sockfd, int udp_sockfd, char* myip, char* user)  //对屏幕输入信息做出判断，若需要发送信息，则存入ipmsg字符串中
{
	while(1)
	{
		char buffer[100];
		fgets(buffer, 100, stdin);
		buffer[strlen(buffer)-1] = '\0';
		
		if(strncmp(buffer, "send to ", strlen("send to ")) == 0)
		{
			int i, j, k;
			char name[10];
			char msg[80];
		
			for(i = 0; i < 10; i++)
			{
				if(buffer[8 + i] == ':')
				{
					name[i] = '\0';
					i++;
					break;
				}
				name[i] = buffer[8 + i];
			}
			
			for(j = 0; j < 80; j++)
			{
				msg[j] = buffer[8 + i + j];
				if(msg[j] == '\0')
					break;
			}
			
			char toip[16];
			User_list* p;
			struct list_head *pos;			
			list_for_each(pos, &head)
			{
				p = list_entry(pos, User_list, list);
				if(strcmp(p->name, name) == 0)
				{
					for(i = 0; i < 16; i++)
						toip[i] = p->ip[i];
					break;
				}
			}
		
			char ipmsg[1024];
			sprintf(ipmsg, "1:%ld:%s:%s:%ld:%s", time(NULL), user, myip, IPMSG_SENDMSG, msg);
			printf("ipmsg:%s\n", ipmsg);
			ipmsg_send(udp_sockfd, toip, ipmsg);    //用udp发送
		}
		/*
		if(strncmp(buffer, "sendfile to ", strlen("sendfile to ")) == 0)
		{
			int i, j, k;
			char name[10];
			char msg[80];
			
			for(i = 0; i < 10; i++)
			{
				if(buffer[12 + i] == ':')
				{
					name[i] = '\0';
					i++;
					break;
				}
				name[i] = buffer[12 + i];
			}
			
			for(j = 0; j < 90; j++)
			{
				msg[j] = buffer[12 + i + j];
				if(msg[j] == '\0')
					break;
			}
			
			ipmsg_send();   //先用udp发送文件属性，申请对方接受
			tcp_send();		//用tcp发送文件
		}
		*/
		if(strncmp(buffer, "user", strlen("user")) == 0)     //打印用户链表
			list_printf();

		if(strncmp(buffer, "clear", strlen("clear")) == 0)   //清屏
			system("clear");
		
		if(strncmp(buffer, "help", strlen("help")) == 0)     //调出help界面
			help_printf();
		
		if(strncmp(buffer, "exit", strlen("exit")) == 0)     //下线
		{
			ipmsg_checkout(bc_sockfd, myip, user);
			exit(0);
		}
	}
}

//tcp_send;
//tcp_recv;
