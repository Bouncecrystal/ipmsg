/*
	File		:error.c
	Description	:��������ʵ��
*/


#include "common.h"

#include <errno.h>
#include <stdarg.h>

/*�ײ�ӿڣ���ӡ�û���ʾ��Ϣ����errnoflagȷ���Ƿ��ӡerrorָ����ϵͳ��ʾ��Ϣ*/
static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
	char buf[MAXLINE];
	
	vsnprintf(buf, MAXLINE - 1, fmt, ap); /*����β���ȫ�������buf������*/
	
	if(errnoflag)	/*����errnoflagȷ���Ƿ���ӱ�׼������Ϣ*/
		snprintf(buf + strlen(buf), MAXLINE - strlen(buf) - 1, ":%s", strerror(error));
	
	strcat(buf, "\n");
	fflush(stdout); /*�Է���׼����ͱ�׼���������ͬһ�������*/
	fputs(buf, stderr);
	fflush(NULL); /*flush���е������*/
}

/*ϵͳ������صķ��������󣬴�ӡ�û���ʾ��Ϣ��ϵͳ��ʾ��Ϣ��Ȼ�󷵻�*/
void err_ret(const char *msg, ...)
{
	va_list ap;
	
	va_start(ap, msg);
	err_doit(1, errno, msg, ap);
	va_end(ap);
}

/*ϵͳ������ص��������󣬴�ӡ�û���ʾ��Ϣ��ϵͳ��ʾ��Ϣ��Ȼ��ֱ�ӵ���exit()��ֹ����*/
void err_sys(const char *msg, ...)
{
	va_list ap;
	
	va_start(ap, msg);
	err_doit(1, errno, msg, ap);
	va_end(ap);
	exit(1);
}

/*ϵͳ������ص��������󣬴�ӡ�û���ʾ��Ϣ��ϵͳ��ʾ��Ϣ����������ת���ļ���Ȼ�����exit()��ֹ����*/
void err_dump(const char *msg, ...)
{
	va_list ap;
	
	va_start(ap, msg);
	err_doit(1, errno, msg, ap);
	va_end(ap);
	abort();  /*��������ת���ļ�CoreDump*/
	exit(1);
}

/*ϵͳ�����޹صķ��������󣬴�ӡ�û���ʾ��Ϣ��Ȼ�󷵻�*/
void err_msg(const char *msg, ...)
{
	va_list ap;
	
	va_start(ap, msg);
	err_doit(0, 0, msg, ap);
	va_end(ap);
}

/*ϵͳ�����޹صķ��������󣬴�ӡ�û���ʾ��Ϣ��errorָ����ϵͳ��ʾ��Ϣ��Ȼ�󷵻�*/
void err_cont(int error, const char *msg, ...)
{
	va_list ap;
	
	va_start(ap, msg);
	err_doit(1, error, msg, ap);
	va_end(ap);
}

/*ϵͳ�����޹ص��������󣬴�ӡ�û���ʾ��Ϣ��Ȼ�����exit()��ֹ����*/
void err_quit(const char *msg, ...)
{
	va_list ap;
	
	va_start(ap, msg);
	err_doit(0, 0, msg, ap);
	va_end(ap);
	exit(1);
}

/*ϵͳ�����޹ص��������� ��ӡ�û���ʾ��Ϣ��errorָ����ϵͳ��ʾ��Ϣ��Ȼ�����exit()��ֹ����*/
void err_exit(int error, const char *msg, ...)
{
	va_list ap;
	
	va_start(ap, msg);
	err_doit(1, error, msg, ap);
	va_end(ap);
	exit(1);
}




















