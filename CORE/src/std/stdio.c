#include <stdio.h>
#include <std/string.h>
#include <vadefs.h>
#include <stdarg.h>
#include <intrinsic.h>
#include <interrupt/syscall.h>
#include <console.h>

int __cdecl printf(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	WORD cs = __getcs();
	int r;
	if (!(cs & 3))
		r = vprintf(SIMPLE_TEXT.COLOR, fmt, va);
	else
	{
		SYSCALL_PRINTF arg;
		arg.TYPE = SYSCALL_TYPE_PRINTF;
		arg.FMRT = fmt;
		arg.VARG = va;
		arg.ATTR = 0x0F;
		r = (int) __syscall(&arg);
	}
	va_end(va);
	return r;
}
int __cdecl cprintf(BYTE attr, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	WORD cs = __getcs();
	int r;
	if (!(cs & 3))
		r = vprintf(attr, fmt, va);
	else
	{
		SYSCALL_PRINTF arg;
		arg.TYPE = SYSCALL_TYPE_PRINTF;
		arg.FMRT = fmt;
		arg.VARG = va;
		arg.ATTR = attr;
		r = (int) __syscall(&arg);
	}
	va_end(va);
	return r;
}