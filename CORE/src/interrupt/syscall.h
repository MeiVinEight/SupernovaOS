#pragma once

#include <types.h>
#include <stddef.h>

#define SYSCALL_TYPE_PRINTF 0x01

typedef struct _SYSCALL_PRINTF
{
	QWORD       TYPE;
	const char *FMRT;
	va_list     VARG;
	DWORD       ATTR;
} SYSCALL_PRINTF;

void setup_system_call();