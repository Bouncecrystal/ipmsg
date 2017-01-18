#include "../include/common.h"

#define ETH_NAME "ens38"

extern MYSQL mysql;  //引入全局mysql描述符
extern struct list_head head;  //引入main.c中全局变量用户链表表头
extern struct list_head sendhead;
extern struct list_head recvhead;
extern int filerecv;  //引入全局文件接收判断字

typedef struct tcp_send_temp  //tcp发送线程传参结构体
{
	char name[10];
	char toip[16];
	char filename[50];
}Tcp_send_temp;

typedef struct tcp_recv_temp  //tcp发送线程传参结构体
{
	char name[10];
	int connfd;
}Tcp_recv_temp;

typedef struct file_head  //定义tcp协议文件头
{
	char name[100];
	int len;//文件长度
	int type;
}File_head;

typedef struct ipmsg  //定义ipmsg解析结构体
{
	char name[10];
	char machine[16];
	char command[33];
	char buffer[1024];
}Ipmsg;

typedef struct user_list   //定义用户链表
{
	char name[10];
	char machine[16];
	char ip[16];
	struct list_head list;
}User_list;

typedef struct tcp_send_list  //定义tcp发送用户链表
{
	char name[10];
	int sockfd;
	struct list_head list;
}Tcp_send_list;

typedef struct tcp_recv_list  //定义tcp接收用户链表
{
	char name[10];
	int connfd;
	struct list_head list;
}Tcp_recv_list;

static void *tcp_send_fun(void* temp);  //tcp发送线程函数
static int fileordir(char* name, int len);  //文件与文件夹名解析函数
static off_t file_len(int fd);  //文件偏移计算函数
static void list_printf();  //用户链表遍历函数
static void help_printf();  //打印help界面
static void ipmsg_analyze(char* msg, Ipmsg* ipmsg);  //ipmsg解析函数
static void ipmsg_checkin(int sockfd, char* ip, char* user);
static void ipmsg_checkout(int sockfd, char* ip, char* user);
static void ipmsg_send(int sockfd, char* toip, char* ipmsg);
static void tcp_send(char* name, char* toip, char* filename);
static void *connect_fun(void *temp);
static void insert_db(MYSQL* mysql, char* senduser, char* recvuser, char* sendip, char* recvip, char* type, char* content);
static void select_db_all(MYSQL* mysql, char* user);
static void select_db_one(MYSQL* mysql, char* user, char* name);

void get_my_ip(int sockfd, char * my_ip);
void bc_recv(int bc_sockfd,int udp_sockfd, char* myip, char* user);
void ipmsg_recv(int bc_sockfd, int udp_sockfd, int tcp_sockfd, char* myip, char* user);
void tcp_recv(int sockfd, char* myip);
void myscanf(int bc_sockfd, int udp_sockfd, int tcp_sockfd, char* myip, char* user);

static void *tcp_send_fun(void* temp)  ////tcp发送线程函数
{
	Tcp_send_temp t = *(Tcp_send_temp*)temp;
	printf("start to sendfile\n");
	tcp_send(t.name, t.toip, t.filename);
}

static int fileordir(char* name, int len)  //文件与文件夹名解析函数
{
	int i = 0, flag = 0;
	char buffer[50];
	if(strncmp(name, "file", 4) == 0)
	{
		flag = 1;
		while(name[6 + i] != ']')
		{
			buffer[i] = name [6 + i];
			i++;
		}
		buffer[i] = '\0';
		bzero(name, len); 
		strncpy(name, buffer, i+1);
	}
	if(strncmp(name, "dir", 3) == 0)
	{
		flag = 2;
		while(name[5 + i] != ']')
		{
			buffer[i] = name [5 + i];
			i++;
		}
		buffer[i] = '\0';
		bzero(name, len);
		strncpy(name, buffer, i+1);
	}
	return flag;
}

static off_t file_len(int fd)   //文件偏移计算函数
{
	off_t cur;
	off_t length;
	cur = lseek(fd, 0, SEEK_CUR);
	lseek(fd, 0, SEEK_SET);
	length = lseek(fd, 0, SEEK_END);
	lseek(fd, cur, SEEK_SET);
	return length;
}

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
	printf("* send to [username]:                      :发送消息            *\n\n");
	printf("* sendfile to [username]:[filename]        :发送文件            *\n\n");
	printf("* cancel sendto [username]                 :取消文件发送        *\n\n");
	printf("* cancel recvfrom [username]               :取消文件接收        *\n\n");
	printf("* user                                     :打印用户列表        *\n\n");
	printf("* updateuser                               :更新用户链表        *\n\n");
	printf("* ls filesend                              :查看发送文件夹      *\n\n");
	printf("* ls filerecv                              :查看接收文件夹      *\n\n");
	printf("* history                                  :查看所有消息记录    *\n\n");
	printf("* history with [username]                  :查看与某用户消息记录*\n\n");
	printf("* clear                                    :清屏                *\n\n");
	printf("* help                                     :帮助                *\n\n");
	printf("* exit                                     :退出                *\n\n");
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
	int ip1, ip2, ip3, ip4;
	char bcip[16];
	sscanf(myip, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
	sprintf(bcip, "%d.%d.%d.255", ip1, ip2, ip3);

	char ipmsg[1024];
	sprintf(ipmsg, "1:%ld:%s:%s:%ld:", time(NULL), user, myip, IPMSG_BR_ENTRY);
	printf("entry msg:%s\n", ipmsg);
	ipmsg_send(sockfd, bcip, ipmsg);
}

static void ipmsg_checkout(int sockfd, char* myip, char* user)   //用户下线广播
{
	int ip1, ip2, ip3, ip4;
	char bcip[16];
	sscanf(myip, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
	sprintf(bcip, "%d.%d.%d.255", ip1, ip2, ip3);
	
	char ipmsg[1024];
	sprintf(ipmsg, "1:%ld:%s:%s:%ld:", time(NULL), user, myip, IPMSG_BR_EXIT);
	printf("exit msg:%s\n", ipmsg);
	ipmsg_send(sockfd, bcip, ipmsg);
}

static void ipmsg_send(int sockfd, char* toip, char* ipmsg)   //用udp发送ipmsg包
{
	int ret;
	
	struct sockaddr_in toaddr;
	toaddr.sin_family = AF_INET;
	toaddr.sin_port = htons(2425);
	inet_pton(AF_INET, toip, &toaddr.sin_addr);

	ret = sendto(sockfd, ipmsg, strlen(ipmsg), 0, (struct sockaddr*)&toaddr, sizeof(toaddr));
	if(ret < 0)
		err_sys("sendto error");
}

static void tcp_send(char* name, char* toip, char* filename)   //用tcp发送文件
{
	int sockfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
		err_sys("tcp send socket error");
	
	//指定接收端地址
	struct sockaddr_in dst_addr;
	bzero(&dst_addr, sizeof(dst_addr));
	dst_addr.sin_family = AF_INET;
	dst_addr.sin_port = htons(9999);
	inet_pton(AF_INET, toip, &dst_addr.sin_addr);

	//发起connect操作
	int ret;
	ret = connect(sockfd, (struct sockaddr*)&dst_addr, sizeof(dst_addr));
	if(ret != 0)
		err_sys("connect error");
	
	Tcp_send_list *tmp = (Tcp_send_list *)malloc(sizeof(Tcp_send_list));
	ASSERT(tmp);
	strncpy(tmp->name, name, strlen(name));
	tmp->sockfd = sockfd;
	list_insert(&tmp->list,&sendhead);
	
	//发送文件
	File_head head;	
	char filepath[50];
	struct stat filestat;
	bzero(filepath, sizeof(filepath));
	bzero(&filestat, sizeof(filestat));
	sprintf(filepath, "./filesend/%s", filename);
	ret = stat(filepath, &filestat);
	if(ret != 0)
		err_sys("stat error");
	
	if(S_ISREG(filestat.st_mode))  //文件
	{	
		bzero(&head, sizeof(head));
		strncpy(head.name, filename, strlen(filename));
		head.len = filestat.st_size;
		head.type = 1;
		
		//发送文件头
		ret = writen(sockfd, &head, sizeof(head));
		if(ret != sizeof(head))
			err_sys("write error");
		
		int fd;
		fd = open(filepath, O_RDONLY);
		if(fd < 0)
			err_sys("open file error");
		
		//发送文件内容
		int n;
		char buffer[1024];
		while((n = read(fd, buffer, 1024)) > 0)
			writen(sockfd, buffer, n);
		
		printf("send %s done\n", head.name);
		close(fd);
	}
	else   //文件夹
	{
		DIR* dir;
		struct dirent* p;
		dir = opendir(filepath);
		if(dir == NULL)
			err_sys("opendir error");
		
		bzero(&head, sizeof(head));
		strncpy(head.name, filename, strlen(filename));
		head.len = filestat.st_size;
		head.type = 2;
		
		//发送文件夹名
		ret = writen(sockfd, &head, sizeof(head));
		if(ret != sizeof(head))
			err_sys("write error");
		char dirname[50];
		bzero(dirname, sizeof(dirname));
		strncpy(dirname, head.name, strlen(head.name));
		
		while(1)
		{
			p = readdir(dir);
			if(p == NULL)
				break;
			if(p->d_ino == -1)
				err_sys("readdir error");
			else if(strncmp(p->d_name, ".", strlen(".")) == 0)
				continue;
			else
			{
				char newfilepath[50];
				bzero(newfilepath, sizeof(newfilepath));
				sprintf(newfilepath, "%s/%s", filepath, p->d_name);
				
				int fd;
				fd = open(newfilepath, O_RDONLY);
				if(fd < 0)
					err_sys("open file error");
				
				bzero(&head, sizeof(head));
				strncpy(head.name, p->d_name, strlen(p->d_name));
				head.len = file_len(fd);
				head.type = 1;
				
				
				//发送文件头
				ret = writen(sockfd, &head, sizeof(head));
				if(ret != sizeof(head))
					err_sys("write error");

				
				//发送文件内容
				int n;
				char buffer[1024];
				while((n = read(fd, buffer, 1024)) > 0)
					writen(sockfd, buffer, n);
				
				printf("send %s done\n", head.name);
				close(fd);
			}
		}
		bzero(&head, sizeof(head));
		head.type = 0;
				
		//发送文件头
		ret = writen(sockfd, &head, sizeof(head));
		if(ret != sizeof(head))
			err_sys("write error");
				
		int fd;
		fd = open(filepath, O_RDONLY);
		if(fd < 0)
			err_sys("open file error");
		
		printf("send dir[%s] done\n", dirname);
		closedir(dir);
	}
	Tcp_send_list* p;
	struct list_head *pos;
	struct list_head *n;
	list_for_each_safe(pos, n, &sendhead)
	{
		p = list_entry(pos, Tcp_send_list, list);
		if(strcmp(p->name, name) == 0)
		{
			list_del(&p->list);  //删除用户
			free(p);
		}
	}
	close(sockfd);
}

static void *connect_fun(void *temp)  //tcp线程
{
		Tcp_recv_temp t = *(Tcp_recv_temp*)temp;
		int connfd = t.connfd;
		
		Tcp_recv_list *tmp = (Tcp_recv_list *)malloc(sizeof(Tcp_recv_list));
		ASSERT(tmp);
		strncpy(tmp->name, t.name, strlen(t.name));
		tmp->connfd = connfd;
		list_insert(&tmp->list,&recvhead); 
		
		char filepath[50];
		File_head head;
		bzero(filepath, sizeof(filepath));
		bzero(&head, sizeof(head));
		
		int n;
		n = readn(connfd, &head, sizeof(head));
		if(n != sizeof(head))
			err_sys("read file head error");
		
		sprintf(filepath, "./filerecv/%s", head.name);
		
		if(head.type == 1)   //文件
		{		
			printf("new file, name:%s, length:%d\n", head.name, head.len);
			
			//接收文件
			int fd;
			fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
			if(fd < 0)
				err_sys("open file error");
			char buffer[1024];
			int left = head.len;
			while(left > 0)
			{
				if(left > 1024)
					n = readn(connfd, buffer, 1024);
				else
					n = readn(connfd, buffer, left);
				write(fd, buffer, n);
				left = left - n;
			}
			
			printf("recv %s done\n", head.name);
			close(fd);
		}
		else if(head.type == 2)  //文件夹
		{
			printf("new dir, name:%s, length:%d\n", head.name, head.len);	
			
			char newfilepath[50];
			char dirname[50];
			bzero(dirname, sizeof(dirname));
			strncpy(dirname, head.name, strlen(head.name));
			
			while(1)
			{
				bzero(&head, sizeof(head));
				bzero(newfilepath, sizeof(newfilepath));
				
				n = readn(connfd, &head, sizeof(head));
				if(n != sizeof(head))
				err_sys("read file head error");
				
				if(head.type == 0)
					break;
				
				printf("new file, name:%s, length:%d\n", head.name, head.len);
				sprintf(newfilepath, "%s/%s", filepath, head.name);
				
				//接收文件
				int fd;
				fd = open(newfilepath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
				if(fd < 0)
					err_sys("open file error");
				char buffer[1024];
				int left = head.len;
				while(left > 0)
				{
					if(left > 1024)
						n = readn(connfd, buffer, 1024);
					else
						n = readn(connfd, buffer, left);
					write(fd, buffer, n);
					left = left - n;
				}
				printf("recv %s done\n", head.name);
				close(fd);
			}
			printf("recv dir[%s] done\n", dirname);
		}
		Tcp_recv_list* p;
		struct list_head *pos;
		struct list_head *nxt;
		list_for_each_safe(pos, nxt, &recvhead)
		{
			p = list_entry(pos, Tcp_recv_list, list);
			if(strcmp(p->name, t.name) == 0)
			{
				list_del(&p->list);  //删除用户
				free(p);
			}
		}
		close(connfd);
}

void get_my_ip(int sockfd, char * my_ip)
{
	struct sockaddr_in sin;
	struct ifreq ifr;
	strncpy(ifr.ifr_name, ETH_NAME, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;
	ioctl(sockfd,SIOCGIFADDR,&ifr);
	memcpy(&sin,&ifr.ifr_addr,sizeof(sin));
	strcpy(my_ip,inet_ntoa(sin.sin_addr));
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
	myaddr.sin_port   = htons(2425); 
	inet_pton(AF_INET, "192.168.31.255", &myaddr.sin_addr);
	
	ret = bind(bc_sockfd, (struct sockaddr*)&myaddr, sizeof(myaddr));
	if(ret != 0)
		err_sys("bind error in ipmsg recv");
	
	ipmsg_checkin(bc_sockfd, myip, user);
	insert_db(&mysql, user, "broadcast", myip, "192.168.31.255", "bc", "checkin");
	
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(2425);
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

			list_insert(&tmp->list,&head);    //添加至用户列表
			if(strcmp(ipmsg.name, user) != 0)
			{
				sprintf(sendipmsg, "1:%ld:%s:%s:%ld:", time(NULL), user, myip, IPMSG_ANSENTRY);
				ipmsg_send(udp_sockfd, ip, sendipmsg);
				printf("ipmsg send:%s\n", sendipmsg);
			}
		}
		
		if(atol(ipmsg.command) == IPMSG_BR_EXIT)  //收到下线通知，删除用户
		{
			printf("%s exit\n", ipmsg.name);

			User_list* p;
			struct list_head *pos;
			struct list_head *n;
			list_for_each_safe(pos, n, &head)
			{
				p = list_entry(pos, User_list, list);
				if(strcmp(p->name, ipmsg.name) == 0)
				{
					list_del(&p->list);  //删除用户
					free(p);
					break;
				}
			}
		}
	}
}

void ipmsg_recv(int bc_sockfd, int udp_sockfd, int tcp_sockfd_send, char* myip, char* user)  //用udp接受ipmsg包
{
	int ret;

	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(2425); 
	inet_pton(AF_INET, myip, &myaddr.sin_addr);
	
	ret = bind(udp_sockfd, (struct sockaddr*)&myaddr, sizeof(myaddr));
	if(ret != 0)
		err_sys("bind error in ipmsg recv");
	
	struct sockaddr_in addr;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(2425);
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
				list_insert(&tmp->list,&head); 
			}
			else
			{
				ipmsg_checkout(bc_sockfd, myip, user); //广播下线
				
				int ip1, ip2, ip3, ip4;
				char bcip[16];
				sscanf(myip, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
				sprintf(bcip, "%d.%d.%d.255", ip1, ip2, ip3);
				
				insert_db(&mysql, user, "broadcast", myip, bcip, "bc", "fail to checkin");
				printf("user already exist\n");
				exit(0);
			}
		}
		
		if(atol(ipmsg.command) == IPMSG_SENDMSG)    //收到消息
		{
			printf("recv from %s:%s\n", ipmsg.name, ipmsg.buffer);
		}
		
		if(atol(ipmsg.command) == IPMSG_RECVMSG)    //收到文件发送确认
		{
			if(ipmsg.buffer[0] != '\0')
			{
				Tcp_send_temp temp;
				bzero(&temp, sizeof(temp));
				strncpy(temp.name, ipmsg.name, strlen(ipmsg.name));
				strncpy(temp.toip, ip, strlen(ip));
				strncpy(temp.filename, ipmsg.buffer, strlen(ipmsg.buffer));
				
				pthread_t tcp_send_pid;
				pthread_create(&tcp_send_pid, NULL, tcp_send_fun, (void*)&temp);
				pthread_detach(tcp_send_pid);
			}
			else
				printf("%s refuse\n", ipmsg.name);
		}
		if(atol(ipmsg.command) == IPMSG_GETFILEDATA)  //收到文件发送请求
		{
				int i = 0;
				filerecv = 0;
				printf("Do you want to recv %s from %s? (Y/N)\n", ipmsg.buffer, ipmsg.name);
				for(i = 0; i<20 ; i++)  //20后放弃接受
				{
					sleep(1);
					if(filerecv != 0)
						break;
				}
				if(i < 20)
				{
					char fileinfo[100];
					bzero(fileinfo, sizeof(fileinfo));
					strcpy(fileinfo, ipmsg.buffer);
					if(filerecv == 1)
					{
							if(fileordir(ipmsg.buffer, strlen(ipmsg.buffer)) == 2)   //文件夹
							{
								char dirpath[40];
								char mkdir[50];
								bzero(dirpath, sizeof(dirpath));
								bzero(mkdir, sizeof(mkdir));
								sprintf(dirpath, "./filerecv/%s", ipmsg.buffer);
								if(access(dirpath, F_OK) != 0)
								{
									sprintf(mkdir, "mkdir %s", dirpath);
									system(mkdir);
								}
								insert_db(&mysql, ipmsg.name, user, ip, myip, "dir", fileinfo);
							}
							else
								insert_db(&mysql, ipmsg.name, user, ip, myip, "file", fileinfo);
							
							if(ret != 0)
								err_sys("mutex unlock error");					
							sprintf(sendipmsg, "1:%ld:%s:%s:%ld:%s", time(NULL), user, myip, IPMSG_RECVMSG, ipmsg.buffer);
							printf("ipmsg send:%s\n", sendipmsg);
							ipmsg_send(udp_sockfd, ip, sendipmsg);    //用udp发送		
					}
					if(filerecv == 2)
					{
						sprintf(sendipmsg, "1:%ld:%s:%s:%ld", time(NULL), user, myip, IPMSG_RECVMSG);
						printf("ipmsg send:%s\n", sendipmsg);
						ipmsg_send(udp_sockfd, ip, sendipmsg);    //用udp发送
					}
				}
				else
				{
					sprintf(sendipmsg, "1:%ld:%s:%s:%ld", time(NULL), user, myip, IPMSG_RECVMSG);
					printf("ipmsg send:%s\n", sendipmsg);
					ipmsg_send(udp_sockfd, ip, sendipmsg);    //用udp发送
					printf("sendfile time out\n");
				}
		}
		
		if(atol(ipmsg.command) == IPMSG_RELEASEFILES)  //收到取消文件发送请求
		{
			printf("%s cancel file transport\n", ipmsg.name);
		}
	}
}

void tcp_recv(int sockfd, char* myip)  //用tcp接收文件
{
	//指定绑定的地址结构
	struct sockaddr_in my_addr;
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(9999);
	inet_pton(AF_INET, myip, &my_addr.sin_addr);
	
	//绑定本址IP地址与端口
	int ret;
	ret = bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr));
	if(ret != 0)
		err_sys("bind error");

	//开始监听
	ret = listen(sockfd, 5);
	if(ret != 0)
		err_sys("listen error");

	while(1)
	{
		struct sockaddr_in client_addr;
		char client_ip[16] = {0};
		socklen_t cliaddr_len = sizeof(client_addr);
		int connfd;

		//接受一个连接请求
		connfd = accept(sockfd, (struct sockaddr*)&client_addr, &cliaddr_len);//阻塞
		if(connfd < 0)
			err_sys("accept error");
		
		//打印对端的IP地址和端口号
		inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, 16);
		printf("client ip=%s, port=%d\n", client_ip, ntohs(client_addr.sin_port));

		Tcp_recv_temp temp;
		temp.connfd = connfd;
		User_list* p;
		struct list_head *pos;			
		list_for_each(pos, &head)
		{
			p = list_entry(pos, User_list, list);
			if(strcmp(p->ip, client_ip) == 0)
			{
				strncpy(temp.name, p->name, strlen(p->name));
				break;
			}
		}
		
		pthread_t pid;
		pthread_create(&pid, NULL, connect_fun, (void*)&temp);
		pthread_detach(pid);
	}
}

void myscanf(int bc_sockfd, int udp_sockfd, int tcp_sockfd, char* myip, char* user)  //对屏幕输入信息做出判断，若需要发送信息，则存入ipmsg字符串中
{
	int ret;
	char ipmsg[1024];
	char buffer[100];
	
	while(1)
	{
		bzero(ipmsg, sizeof(ipmsg));
		bzero(buffer, sizeof(buffer));
		
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
			
			for(j = 0; j < 92; j++)
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
		
			sprintf(ipmsg, "1:%ld:%s:%s:%ld:%s", time(NULL), user, myip, IPMSG_SENDMSG, msg);
			printf("ipmsg send:%s\n", ipmsg);
			ipmsg_send(udp_sockfd, toip, ipmsg);    //用udp发送
			insert_db(&mysql, user, name, myip, toip, "msg", msg);
		}

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
			
			for(j = 0; j < 88; j++)
			{
				msg[j] = buffer[12 + i + j];
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
			
			char filepath[50];
			struct stat filestat;
			bzero(filepath, sizeof(filepath));
			bzero(&filestat, sizeof(filestat));

			sprintf(filepath, "./filesend/%s", msg);
			if(access(filepath, F_OK) == 0)
			{
				ret = stat(filepath, &filestat);
				if(ret != 0)
					err_sys("stat error");
				if(S_ISREG(filestat.st_mode))
				{	
					sprintf(ipmsg, "1:%ld:%s:%s:%ld:file [%s][%ld]", time(NULL), user, myip, IPMSG_GETFILEDATA, msg, filestat.st_size);
					printf("ipmsg send:%s\n", ipmsg);
					ipmsg_send(udp_sockfd, toip, ipmsg);    //用udp发送
				}
				else if(S_ISDIR(filestat.st_mode))
				{
					sprintf(ipmsg, "1:%ld:%s:%s:%ld:dir [%s][%ld]", time(NULL), user, myip, IPMSG_GETFILEDATA, msg, filestat.st_size);
					printf("ipmsg send:%s\n", ipmsg);
					ipmsg_send(udp_sockfd, toip, ipmsg);    //用udp发送
				}
				else
					printf("it's not neither a file nor a dir\n");
			}
			else
				printf("file not exist\n");
		}
		
		if(strncmp(buffer, "Y", strlen("Y")) == 0)  //接受文件接收
			filerecv = 1;
		if(strncmp(buffer, "N", strlen("N")) == 0)  //拒绝文件接受
			filerecv = 2;
		
		if(strncmp(buffer, "user", strlen("user")) == 0)     //打印用户链表
			list_printf();
		
		if(strncmp(buffer, "updateuser", strlen("updateuser")) == 0)  //更新用户链表
		{
			User_list* p;
			struct list_head *pos;
			struct list_head *n;
			list_for_each_safe(pos, n, &head)
			{
				p = list_entry(pos, User_list, list);
				list_del(&p->list);  //删除用户
				free(p);
			}
			ipmsg_checkout(bc_sockfd, myip, user); //广播下线
			ipmsg_checkin(bc_sockfd, myip, user); //广播上线
			sleep(1);
			list_printf();
		}
		
		if(strncmp(buffer, "clear", strlen("clear")) == 0)   //清屏
			system("clear");
		
		if(strncmp(buffer, "help", strlen("help")) == 0)     //调出help界面
			help_printf();
		
		if(strncmp(buffer, "exit", strlen("exit")) == 0)     //下线
		{
			ipmsg_checkout(bc_sockfd, myip, user);
			insert_db(&mysql, user, "broadcast", myip, "192.168.31.255", "bc", "checkout");
			close(tcp_sockfd);
			close(udp_sockfd);
			close(bc_sockfd);
			mysql_close(&mysql);
			exit(0);
		}
		
		if(strncmp(buffer, "ls filesend", strlen("ls filesend") - 1) == 0)    //查看发送目录
		{
			int i = 0;
			char dirpath[30];
			char command[50];
			bzero(dirpath, sizeof(dirpath));
			bzero(command, sizeof(command));
			while(buffer[11 + i] != '\0')
			{
				dirpath[i] = buffer[11 + i];
				i++;
			}
			dirpath[i] = '\0';
			
			sprintf(command, "ls ./filesend%s", dirpath);
			system(command);
		}
		
		if(strncmp(buffer, "ls filerecv", strlen("ls filerecv") - 1) == 0)    //查看接收目录
		{
			int i = 0;
			char dirpath[30];
			char command[50];
			bzero(dirpath, sizeof(dirpath));
			bzero(command, sizeof(command));
			while(buffer[11 + i] != '\0')
			{
				dirpath[i] = buffer[11 + i];
				i++;
			}
			dirpath[i] = '\0';
			
			sprintf(command, "ls ./filerecv%s", dirpath);
			system(command);
		}
		
		if(strncmp(buffer, "history", strlen("history")) == 0)   //查看聊天记录
		{
			if(strncmp(buffer, "history with ", strlen("history with ")) == 0)   //查看与某用户聊天记录
			{
				char name[10];
				bzero(name, sizeof(name));
				
				int i = 0;
				while(buffer[13 + i] != '\0')
				{
					name[i] = buffer[13 + i];
					i++;
				}
				name[i] = '\0';
				
				select_db_one(&mysql, user, name);
			}
			else
				select_db_all(&mysql, user);  //查看当前用户所有聊天记录
		}
		
		if(strncmp(buffer, "cancel sendto ", strlen("cancel sendto ")) == 0)    //取消文件发送
		{
			char name[10];
			bzero(name, sizeof(name));
				
			int i = 0;
			while(buffer[14 + i] != '\0')
			{
				name[i] = buffer[14 + i];
				i++;
			}
			name[i] = '\0';
			
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
			
			Tcp_send_list* q;
			struct list_head *post;
			struct list_head *next;
			list_for_each_safe(post, next, &sendhead)
			{
				q = list_entry(post, Tcp_send_list, list);
				if(strcmp(q->name, name) == 0)
				{
					close(q->sockfd);
					list_del(&q->list);  //删除用户
					free(q);
				}
			}
			sprintf(ipmsg, "1:%ld:%s:%s:%ld:", time(NULL), user, myip, IPMSG_RELEASEFILES);
			printf("ipmsg send:%s\n", ipmsg);
			ipmsg_send(udp_sockfd, toip, ipmsg);    //用udp发送
			insert_db(&mysql, user, name, myip, toip, "cel", "cancel filesend");
			printf("cancel done\n");
		}
		
		if(strncmp(buffer, "cancel recvfrom ", strlen("cancel recvfrom ")) == 0)    //取消文件接收
		{
			char name[10];
			bzero(name, sizeof(name));
				
			int i = 0;
			while(buffer[16 + i] != '\0')
			{
				name[i] = buffer[16 + i];
				i++;
			}
			name[i] = '\0';
			
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
			
			Tcp_recv_list* q;
			struct list_head *post;
			struct list_head *next;
			list_for_each_safe(post, next, &sendhead)
			{
				q = list_entry(post, Tcp_recv_list, list);
				if(strcmp(q->name, name) == 0)
				{
					close(q->connfd);
					list_del(&q->list);  //删除用户
					free(q);
				}
			}
			sprintf(ipmsg, "1:%ld:%s:%s:%ld:", time(NULL), user, myip, IPMSG_RELEASEFILES);
			printf("ipmsg send:%s\n", ipmsg);
			ipmsg_send(udp_sockfd, toip, ipmsg);    //用udp发送
			insert_db(&mysql, user, name, myip, toip, "cel", "cancel filerecv");
			printf("cancel done\n");
		}
	}
}

void insert_db(MYSQL* mysql, char* senduser, char* recvuser, char* sendip, char* recvip, char* type, char* content)
{
	int n;
	char str[1024];
	bzero(str, sizeof(str));
	sprintf(str, "insert into record(senduser, recvuser, sendip, recvip, type, content) \
					values('%s', '%s', '%s', '%s', '%s', '%s')", senduser, recvuser, sendip, recvip, type, content);
	n = mysql_query(mysql, str);
	if(n != 0)
		err_sys("mysql_query error");
}

void select_db_all(MYSQL* mysql, char* user)
{
	int n;
	char str[1024];
	bzero(str, sizeof(str));
	sprintf(str, "select * from record where senduser='%s' or recvuser='%s'", user, user);
	n = mysql_query(mysql, str);
	if(n != 0)
		err_sys("mysql_query error");

	MYSQL_RES* record;
	record = mysql_store_result(mysql);
	
	printf("number senduser recvuser    sendip         recvip         type buffer               time\n");
	
	MYSQL_ROW result;
	while(result = mysql_fetch_row(record))
	{
		printf("%-6s %-8s %-11s %s %s %-4s %-20s %s\n", result[0], result[1], result[2], result[3],\
				result[4], result[5], result[6], result[7]);
	}
	mysql_free_result(record);
}

void select_db_one(MYSQL* mysql, char* user, char* name)
{
	int n;
	char str[1024];
	bzero(str, sizeof(str));
	sprintf(str, "select * from record where senduser='%s' and recvuser='%s' or senduser='%s' and recvuser='%s'", user, name, name, user);
	mysql_query(mysql, str);
	if(n != 0)
		err_sys("mysql_query error");

	MYSQL_RES* record;
	record = mysql_store_result(mysql);
	
	printf("number senduser recvuser    sendip         recvip         type buffer               time\n");
	
	MYSQL_ROW result;
	while(result = mysql_fetch_row(record))
	{
		printf("%-6s %-8s %-11s %s %s %-4s %-20s %s\n", result[0], result[1], result[2], result[3],\
				result[4], result[5], result[6], result[7]);
	}
	mysql_free_result(record);
}



