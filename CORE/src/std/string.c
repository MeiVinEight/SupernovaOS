#include <string.h>

/*
char *strcpy(char *dst, const char *src)
{
	if (!src || !dst)
		return 0;
	char *buf = dst;
	while (*src)
	{
		*buf = *src;
		src++;
	}
	return buf;
}
*/
char *strtrim(char *str)
{
	char *ret = str;
	char *buf = str;
	int flag = 1;
	for (; *buf; buf++)
	{
		if (flag && (*buf == 0x20))
			continue;
		flag = 0;
		*str = *buf;
		str++;
	}
	buf = str;
	*buf = 0;
	while (buf > ret)
	{
		--buf;
		if (*buf != 0x20)
			break;
		*buf = 0;
	}
	return ret;
}