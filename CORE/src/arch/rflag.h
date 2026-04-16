#pragma once

#define RFL_IOPL(x) (((x) >> 12) & 3)
#define RFL_NT  (1 << 14)
#define RFL_RF  (1 << 16)
#define RFL_VM  (1 << 17)
#define RFL_AC  (1 << 18)
#define RFL_VIF (1 << 19)
#define RFL_VIP (1 << 20)
#define RFL_ID  (1 << 21)

typedef struct _RFLAGS
{
	DWORD CF:1;
	DWORD R0:1;
	DWORD PF:1;
	DWORD R1:1;
	DWORD AF:1;
	DWORD R2:1;
	DWORD ZF:1;
	DWORD SF:1;
	DWORD TF:1;
	DWORD IF:1;
	DWORD DF:1;
	DWORD OF:1;
	DWORD IOPL:2;
	DWORD NT:1;
	DWORD R3:1;
	DWORD RF:1;
	DWORD VM:1;
	DWORD AC:1;
	DWORD VIF:1;
	DWORD VIP:1;
	DWORD ID:1;
	DWORD R4:10;
} RFLAGS;