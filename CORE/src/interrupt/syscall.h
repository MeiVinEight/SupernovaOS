#pragma once

#include <types.h>
#include <stddef.h>

#define SYSCALL_TYPE_PRINTF        0x01
#define SYSCALL_TYPE_VIRTUAL_ALLOC 0x02

typedef struct _SYSCALL_PRINTF
{
	QWORD       TYPE;
	const char *FMRT;
	va_list     VARG;
	DWORD       ATTR;
} SYSCALL_PRINTF;
typedef struct _SYSCALL_VIRTUAL_ALLOC
{
	QWORD  TYPE;
	QWORD  PROC;
	QWORD *ADDR;
	QWORD  SIZE;
	DWORD  ATYP;
} SYSCALL_VIRTUAL_ALLOC;

void setup_system_call();