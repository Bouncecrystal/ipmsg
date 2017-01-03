#ifndef __COMMON_H
#define __COMMON_H

/*
	File		:common.h
	Description	:�Զ����ͷ�ļ����������õ�ϵͳͷ�ļ������õĺ꣬�Զ���Ĺ��ߺ���
*/

/*ͷ�ļ�ͳһ����*/

/*��׼ͷ�ļ�*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

/*linuxϵͳ����ͷ�ļ�*/
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

/*��Ŀ����ͷ�ļ�*/
#include <ipmsg.h>
#include <list.h>

#define MAXLINE 4096  /*һ�е���󳤶�*/

#define ERR_OK 				0		/*�ɹ�����*/
#define ERR_INVALID_INPUT 	-1   	/*�������Ϸ�*/

/*������Ԫ�ظ����ĺ�*/
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

/*��ȫ�ͷŶ��ڴ�ĺ�*/
#define SAFEFREE(p) do{if(p)free(p);p = NULL;}while(0)

/*��ȫ�ر��ļ�ָ��*/
#define SAFEFCLOSE(p) do {if(p)fclose(p); p = NULL;}while(0)

/*��ȫ�ر��ļ�������*/
#define SAFECLOSE(fd) do{if(fd >= 0) close(fd); fd = -1;}while(0)

/*�Զ���ĵ��Ժ꣬��ӡ�ļ��������������кź��û���ʾ��Ϣ*/
#ifdef DEBUG_ON
#define DEBUG(...) do{\
	printf("[%s][%s][%d]:", __FILE__, __func__, __LINE__);\
	printf(__VA_ARGS__);\
	printf("\n");\
}while(0)
#else
#define DEBUG(...)
#endif

/*�Զ���ASSERT��*/
#define ASSERT(p) do{\
    if(!p){\
        printf("[%s][%s][%d]:Assert Fail!\n", __FILE__, __func__, __LINE__);\
        abort();\
    }\
}while(0)
	
/*ʹ��typedefʵ�ֲ�������*/
typedef enum{false, true}bool;

/*��ȫ���������������ֵ*/
#define MAX(a, b) ({\
	typeof(a) __a = (a);\
	typeof(b) __b = (b);\
	__a > __b ? __a : __b;\
})
/*
    *Function   : get_random_string
    *Description: ��������Ϊlength - 1������ַ���
    *Param in   : length    Ҫ�����ַ����ĳ���
    *Param out  : p    ����ַ���������
    *Return     : ERR_OK �ɹ�����
                  ERR_INVALID_INPUT    ��������
    *Note       :����������ַ���ֻ������ĸ�����֣������������ַ�����\0��β
*/
int get_random_string(char *p, int length);


ssize_t readn(int fd, void *buf, size_t count);
ssize_t writen(int fd, const void *buf, size_t count);

/*�Զ��庯��ԭ��*/
/*��err_��ͷ���Ǵ������������ڳ���ʱ��ӡ��ʾ��Ϣ��������ز���*/

/*ϵͳ������صķ��������󣬴�ӡ�û���ʾ��Ϣ��ϵͳ��ʾ��Ϣ��Ȼ�󷵻�*/
void err_ret(const char *msg, ...);

/*ϵͳ������ص��������󣬴�ӡ�û���ʾ��Ϣ��ϵͳ��ʾ��Ϣ��Ȼ��ֱ�ӵ���exit()��ֹ����*/
void err_sys(const char *msg, ...);

/*ϵͳ������ص��������󣬴�ӡ�û���ʾ��Ϣ��ϵͳ��ʾ��Ϣ����������ת���ļ���Ȼ�����exit()��ֹ����*/
void err_dump(const char *fmt, ...);

/*ϵͳ�����޹صķ��������󣬴�ӡ�û���ʾ��Ϣ��Ȼ�󷵻�*/
void err_msg(const char *msg, ...);

/*ϵͳ�����޹صķ��������󣬴�ӡ�û���ʾ��Ϣ��errorָ����ϵͳ��ʾ��Ϣ��Ȼ�󷵻�*/
void err_cont(int error, const char *msg, ...);

/*ϵͳ�����޹ص��������󣬴�ӡ�û���ʾ��Ϣ��Ȼ�����exit()��ֹ����*/
void err_quit(const char *msg, ...);

/*ϵͳ�����޹ص��������� ��ӡ�û���ʾ��Ϣ��errorָ����ϵͳ��ʾ��Ϣ��Ȼ�����exit()��ֹ����*/
void err_exit(int error, const char *msg, ...);

#endif




















