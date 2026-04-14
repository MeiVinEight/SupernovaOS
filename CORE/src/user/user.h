#pragma once

#include <file/pe32x.h>

typedef struct _USERSPACE_SYSTEM_TABLE
{
	IMAGE_DOS_HEADER DOSX;
	BYTE RSV0[0x0FC0];
	// Offset 0x1000
	BYTE STCK[0x8000];
	BYTE FONT[0x1000];
} USERSPACE_SYSTEM_TABLE;

void user_main();