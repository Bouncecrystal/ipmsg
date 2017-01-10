#include "../include/common.h"

struct list_head head;//定义全局变量用户链表表头

void bc_recv(int bc_sockfd, int udp_sockfd, char* myip, char* user);
void ipmsg_recv(int udp_sockfd, char* myip, char* user);
void myscanf(int bc_sockfd,int udp_sockfd, char* myip, char* user);

typedef struct local_addr //定义本地地址结构
{
	int bc_sockfd;
	int udp_sockfd;
	char user[10];
	char ip[16];
	char port[10];
}Local_addr;

void* bc_fun(void* temp) 
{
	Local_addr addr = *(Local_addr*)temp;
	bc_recv(addr.bc_sockfd, addr.udp_sockfd, addr.ip, addr.user);
}

void* msg_fun(void* temp)  //消息接收判别线程 需要与其他线程通信   
{
	Local_addr addr = *(Local_addr*)temp;
	ipmsg_recv(addr.udp_sockfd, addr.ip, addr.user);   
}

void* file_fun()  //文件接收线程
{
	//tcp_recv();
	while(1){}
}

void* db_fun()   //与数据库连接  实时跟新数据库
{
	while(1){}
}

int main(int argc, char** argv)  //输入本地ip
{
	if(argc != 2)
		err_quit("usage:./ipmsg ip");
	
	INIT_LIST_HEAD(&head);   //初始化链表头
	
	Local_addr local_addr;
	printf("please input username:\n");
	scanf("%s", local_addr.user);
	getchar();
	
	int bc_sockfd, udp_sockfd;
	bc_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(bc_sockfd < 0)
		err_sys("bc socket error");
	udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(udp_sockfd < 0)
		err_sys("udp socket error");
	
	local_addr.bc_sockfd = bc_sockfd;
	local_addr.udp_sockfd = udp_sockfd;
	strcpy(local_addr.ip, argv[1]);
	strcpy(local_addr.port, "2325");
	
	pthread_t bc_pid, msg_pid, file_pid, db_pid;
	pthread_create(&bc_pid, NULL, bc_fun, (void*)&local_addr);
	pthread_create(&msg_pid, NULL, msg_fun, (void*)&local_addr);
	pthread_create(&file_pid, NULL, file_fun, NULL);
	pthread_create(&db_pid, NULL, db_fun, NULL);
	pthread_detach(bc_pid);
	pthread_detach(msg_pid);
	pthread_detach(file_pid);
	pthread_detach(db_pid);

	myscanf(bc_sockfd, udp_sockfd, local_addr.ip, local_addr.user);
	while(1){}
	
	return 0;
}
