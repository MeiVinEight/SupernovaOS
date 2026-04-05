#include <stdio.h>
#include <memory/virtmem.h>
#include <std/string.h>
#include <vadefs.h>
#include <stdarg.h>
#include <console.h>
#include <core.h>

int __cdecl printf(const char* fmt, ...)
{
	QWORD pc = 1;
	QWORD buf = alloc_physical_memory(&pc, 0, 0);
	char *bufx = (char *) core_mapping(buf);
	va_list va;
	va_start(va, fmt);
	int r = vsprintf(bufx, fmt, va);
	va_end(va);
	simple_output(bufx);
	free_physical_memory(buf, 1);
	return r;
}