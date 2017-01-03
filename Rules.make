#######################################################
#���ļ�����ָ�����������ÿ��Makefile�ж���������ļ�
#######################################################

#�Ƿ�֧�ֶδ������
support_sigdebug = y

#�Ƿ�򿪵���ѡ��
support_debug = y

#ָ��������
CC=gcc

#ָ���鵵��������������̬��
AR=ar

#ָ��ͷ�ļ�·��
INCDIR=$(ROOT)/include

#ָ�����ļ�·��
LDDIR=-L$(ROOT)/lib

#ָ������
LIB=-lcommon 

#ָ������ѡ��
CFLAGS=-Wall -g -I$(INCDIR) 

#ָ������ѡ��
#ע�⣺libcommon.a������libdl.a������-lcommonѡ��Ҫλ��-ldlǰ�棨��̬��ı�������ӿ��˳����Ҫ��
LDFLAGS=$(LDDIR) $(LIB) -lpthread

#���֧�ֶδ�����ԣ����������������ѡ��
ifeq ($(support_sigdebug), y)
CFLAGS += -rdynamic
LDFLAGS += -ldl
endif

#����򿪵���ѡ������DEBUG_ON��
ifeq ($(support_debug), y)
CFLAGS += -DDEBUG_ON
endif 

#�������ɵ��м��ļ�������make clean����
TEMPFILES=core core.* *.o temp.* *.out

















