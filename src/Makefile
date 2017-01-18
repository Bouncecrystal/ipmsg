a.out:main.o ipmsg.o
	cc -o a.out main.o ipmsg.o -L../lib -lcommon -lpthread -lmysqlclient

main.o ipmsg.o:main.c ipmsg.c
	cc -c main.c ipmsg.c

clean:
	rm -f a.out *.o
