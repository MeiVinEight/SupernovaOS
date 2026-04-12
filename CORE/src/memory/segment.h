#pragma once
#include <intrinsic.h>

typedef struct _GLOBAL_SEGMENT_DESCRIPTOR
{
	DWORD RSV0;
	BYTE  RSV1;
	BYTE  ACCE:1;
	BYTE  RDWR:1;
	BYTE  DICO:1;
	BYTE  EXEC:1;
	BYTE  SEGT:1;
	BYTE  DPLX:2;
	BYTE  PRST:1;
	BYTE  RSV2:5;
	BYTE  LONG:1;
	BYTE  SIZE:1;
	BYTE  GRAN:1;
	BYTE  RSV3;
} GLOBAL_SEGMENT_DESCRIPTOR;
#pragma pack(push, 2)
typedef struct _GDTR64
{
	WORD L;
	QWORD A;
} GDTR64;
#pragma pack(pop)

void setup_segment();