#pragma once

#include <types.h>

#pragma pack(push, 4)
typedef struct _TASK_STATE_SEGMENT
{
	DWORD RSV0;
	QWORD RSPX[3];
	QWORD RSV1;
	QWORD TSTX[7];
	QWORD RSV2;
	WORD  RSV3;
	WORD  IOMA;
} TASK_STATE_SEGMENT;
#pragma pack(pop)
typedef struct _TSS_DESCRIPTOR
{
	WORD  LIMT;
	WORD  ADD0;
	BYTE  ADD1;
	BYTE  TYPE:4;
	BYTE  ZRO0:1;
	BYTE  DPLX:2;
	BYTE  PRST:1;
	BYTE  ZRO1:4;
	BYTE  AVAL:1;
	BYTE  ZRO2:2;
	BYTE  GRAN:1;
	BYTE  ADD2;
	DWORD ADD3;
	DWORD ZRO3;
} TSS_DESCRIPTOR;

void load_tss(void *tss, void *gdt, WORD off);