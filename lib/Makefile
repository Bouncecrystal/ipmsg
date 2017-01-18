#
# 编译库文件的Makefile
#

#顶层路径
ROOT = $(firstword $(subst ipmsg, ipmsg ,$(PWD)))/ipmsg

#包含编译规则
include $(ROOT)/Rules.make

#编译最终目标
target = libcommon.a

#依赖的目标文件名
OBJS = error.o readn.o

#如果程序要支持段错误调试，则把sigsegv.c加入到库文件中
ifeq ($(support_sigdebug), y)
OBJS += sigsegv.o
endif

all:$(OBJS)
	ar -cvrs $(target) $^

%.o:%.c
	gcc $< $(CFLAGS) -c
	
clean:
	rm -f *.o a.out core temp.* $(target)

