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

void load_tss(void *tss, void *gdt, WORD off);