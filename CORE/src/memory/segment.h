#pragma once
#include <intrinsic.h>

#pragma pack(push, 2)
typedef struct _GDTR64
{
	WORD L;
	QWORD A;
} GDTR64;
#pragma pack(pop)

void setup_segment();