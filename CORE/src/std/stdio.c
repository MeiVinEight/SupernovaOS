#include <stdio.h>
#include <std/string.h>
#include <vadefs.h>
#include <stdarg.h>

int __cdecl printf(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	int r = vprintf(fmt, va);
	va_end(va);
	return r;
}