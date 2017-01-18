#include "../include/common.h"

MYSQL mysql;  //定义全局mysql描述符
struct list_head head; //定义全局变量用户链表表头
struct list_head sendhead;
struct list_head recvhead;
int filerecv; //定义全局文件接收判断字

void get_my_ip(int sockfd, char * my_ip);
void bc_recv(int bc_sockfd, int udp_sockfd, char* myip, char* user);
void ipmsg_recv(int bc_sockfd, int udp_sockfd, int tcp_sockfd, char* myip, char* user);
void tcp_recv(int sockfd, char* myip);
void myscanf(int bc_sockfd, int udp_sockfd, int tcp_sockfd, char* myip, char* user);

typedef struct local_addr //定义本地地址结构
{
	int bc_sockfd;
	int udp_sockfd;
	int tcp_sockfd;
	char user[10];
	char ip[16];
	char port[10];
}Local_addr;

void signal_handler(int signo)   //信号处理函数
{
	printf("\nplease input exit to quit\n");
}

void* bc_fun(void* temp) 
{
	Local_addr addr = *(Local_addr*)temp;
	bc_recv(addr.bc_sockfd, addr.udp_sockfd, addr.ip, addr.user);
}

void* msg_fun(void* temp)  //消息接收判别线程 需要与其他线程通信   
{
	Local_addr addr = *(Local_addr*)temp;
	ipmsg_recv(addr.bc_sockfd, addr.udp_sockfd, addr.tcp_sockfd, addr.ip, addr.user);   
}

void* file_fun(void* temp)  //文件接收线程
{
	Local_addr addr = *(Local_addr*)temp;
	sleep(1);
	tcp_recv(addr.tcp_sockfd, addr.ip);
	while(1){}
}

int main()
{
	signal(SIGINT, signal_handler);  //注册信号处理
	
	INIT_LIST_HEAD(&head);   //初始化链表头
	INIT_LIST_HEAD(&sendhead);  
	INIT_LIST_HEAD(&recvhead);   
	

	if(access("./filesend", F_OK) !=0)
		system("mkdr ./filesend");
	if(access("./filerecv", F_OK) !=0)
		system("mkdr ./filerecv");
	
	
	Local_addr local_addr;
	while(1)
	{
		bzero(&local_addr, sizeof(local_addr));
		printf("please input username:\n");
		scanf("%s", local_addr.user);
		getchar();
		if(local_addr.user[0] == '\0' || local_addr.user[10] != '\0')
			printf("illegal user\n");
		else
			break;
	}
	
	MYSQL* ret;
	ret = mysql_init(&mysql);
	if(ret == NULL)
		err_sys("mysql_init error");

	ret = mysql_real_connect(&mysql, "192.168.31.135", "root", "1", "ipmsg", 0, NULL, 0);
	if(ret == NULL)
		err_sys("mysql_real_connect error");
	
	int bc_sockfd, udp_sockfd, tcp_sockfd;
	bc_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(bc_sockfd < 0)
		err_sys("bc socket error");
	udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(udp_sockfd < 0)
		err_sys("udp socket error");
	tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp_sockfd < 0)
		err_sys("tcp socket error");
	
	local_addr.bc_sockfd = bc_sockfd;
	local_addr.udp_sockfd = udp_sockfd;
	local_addr.tcp_sockfd = tcp_sockfd;
	get_my_ip(bc_sockfd, local_addr.ip);
	strcpy(local_addr.port, "2425");
	
	pthread_t bc_pid, msg_pid, file_pid, db_pid;
	pthread_create(&bc_pid, NULL, bc_fun, (void*)&local_addr);
	pthread_create(&msg_pid, NULL, msg_fun, (void*)&local_addr);
	pthread_create(&file_pid, NULL, file_fun, (void*)&local_addr);
	pthread_detach(bc_pid);
	pthread_detach(msg_pid);
	pthread_detach(file_pid);
	
	myscanf(bc_sockfd, udp_sockfd, tcp_sockfd, local_addr.ip, local_addr.user);
	
	close(tcp_sockfd);
	close(udp_sockfd);
	close(bc_sockfd);
	mysql_close(&mysql);
	
	return 0;
}
