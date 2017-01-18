#include "../include/common.h"
#include <errno.h>
//读取n个字节
ssize_t readn(int fd, void *buf, size_t count)
{
	int n;
	size_t left = count;//剩余长度
	
	while(left > 0)
	{
		n = read(fd, buf + count - left, left);
		if(n < 0)
		{
			if(errno == EINTR)
				continue;
			else
				break;
		}
		left -= n;
	}
	return count - left;
}

ssize_t writen(int fd, const void *buf, size_t count)
{
	int n;
	size_t left = count;//剩余长度
	
	while(left > 0)
	{
		n = write(fd, buf + count - left, left);
		if(n < 0)
		{
			if(errno == EINTR)
				continue;
			else
				break;
		}
		left -= n;
	}
	return count - left;
}