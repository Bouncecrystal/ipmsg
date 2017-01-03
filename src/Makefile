#顶层路径
ROOT = $(firstword $(subst ipmsg, ipmsg ,$(PWD)))/ipmsg

#包含编译规则
include $(ROOT)/Rules.make

#获取当前目录所有的.c文件
CSOURCES = $(wildcard *.c)

#去掉所有文件的.c后缀，作为目标文件名
targets = $(CSOURCES:.c=)

all:$(targets)


%:%.c
	$(CC) $(CFLAGS) $@.c -o $@ $(LDFLAGS)

clean:
	rm -f $(targets) $(TEMPFILES) *.o

