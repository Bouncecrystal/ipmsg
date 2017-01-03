#ifndef __COMMON_H
#define __COMMON_H

/*
	File		:common.h
	Description	:自定义的头文件，包含常用的系统头文件，常用的宏，自定义的工具函数
*/

/*头文件统一包含*/

/*标准头文件*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

/*linux系统调用头文件*/
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

/*项目所需头文件*/
#include <ipmsg.h>
#include <list.h>

#define MAXLINE 4096  /*一行的最大长度*/

#define ERR_OK 				0		/*成功返回*/
#define ERR_INVALID_INPUT 	-1   	/*参数不合法*/

/*求数组元素个数的宏*/
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

/*安全释放堆内存的宏*/
#define SAFEFREE(p) do{if(p)free(p);p = NULL;}while(0)

/*安全关闭文件指针*/
#define SAFEFCLOSE(p) do {if(p)fclose(p); p = NULL;}while(0)

/*安全关闭文件描述符*/
#define SAFECLOSE(fd) do{if(fd >= 0) close(fd); fd = -1;}while(0)

/*自定义的调试宏，打印文件名，函数名，行号和用户提示信息*/
#ifdef DEBUG_ON
#define DEBUG(...) do{\
	printf("[%s][%s][%d]:", __FILE__, __func__, __LINE__);\
	printf(__VA_ARGS__);\
	printf("\n");\
}while(0)
#else
#define DEBUG(...)
#endif

/*自定义ASSERT宏*/
#define ASSERT(p) do{\
    if(!p){\
        printf("[%s][%s][%d]:Assert Fail!\n", __FILE__, __func__, __LINE__);\
        abort();\
    }\
}while(0)
	
/*使用typedef实现布尔类型*/
typedef enum{false, true}bool;

/*安全地求两个数的最大值*/
#define MAX(a, b) ({\
	typeof(a) __a = (a);\
	typeof(b) __b = (b);\
	__a > __b ? __a : __b;\
})
/*
    *Function   : get_random_string
    *Description: 产生长度为length - 1的随机字符串
    *Param in   : length    要产生字符串的长度
    *Param out  : p    随机字符串缓存区
    *Return     : ERR_OK 成功返回
                  ERR_INVALID_INPUT    参数错误
    *Note       :产生的随机字符串只包含字母与数字，不包含特殊字符，以\0结尾
*/
int get_random_string(char *p, int length);


ssize_t readn(int fd, void *buf, size_t count);
ssize_t writen(int fd, const void *buf, size_t count);

/*自定义函数原型*/
/*以err_开头的是错误处理函数，用于出错时打印提示信息并进行相关操作*/

/*系统调用相关的非致命错误，打印用户提示信息和系统提示信息，然后返回*/
void err_ret(const char *msg, ...);

/*系统调用相关的致命错误，打印用户提示信息和系统提示信息，然后直接调用exit()终止程序*/
void err_sys(const char *msg, ...);

/*系统调用相关的致命错误，打印用户提示信息和系统提示信息并产生核心转储文件，然后调用exit()终止程序*/
void err_dump(const char *fmt, ...);

/*系统调用无关的非致命错误，打印用户提示信息，然后返回*/
void err_msg(const char *msg, ...);

/*系统调用无关的非致命错误，打印用户提示信息和error指定的系统提示信息，然后返回*/
void err_cont(int error, const char *msg, ...);

/*系统调用无关的致命错误，打印用户提示信息，然后调用exit()终止程序*/
void err_quit(const char *msg, ...);

/*系统调用无关的致命错误， 打印用户提示信息和error指定的系统提示信息，然后调用exit()终止程序*/
void err_exit(int error, const char *msg, ...);

#endif




















