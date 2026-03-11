#include <memory/segment.h>


COREAPI BYTE LXS0[] =
{
	0x6A, 0x08,                                                 // PUSH 08
	0x51,                                                       // PUSH RCX
	0x48, 0xCB,                                                 // RETFQ
};
COREAPI BYTE LXS1[] =
{
	0x31, 0xC9,                                                 // XOR ECX, ECX
	0x66, 0x8E, 0xE1,                                           // MOV FS, CX
	0x66, 0x8E, 0xE9,                                           // MOV GS, CX
	0x83, 0xC1, 0x10,                                           // ADD ECX, 10
	0x66, 0x8E, 0xC1,                                           // MOV ES, CX
	0x66, 0x8E, 0xD1,                                           // MOV SS, CX
	0x66, 0x8E, 0xD9,                                           // MOV DS, CX
	0xC3,                                                       // RET
};

void setup_segment()
{
	// Setup GDT
	SYSTEM_TABLE->GDT[0] = 0x0000000000000000ULL; // Null Segment
	SYSTEM_TABLE->GDT[1] = 0x00209A0000000000ULL; // Code Segment
	SYSTEM_TABLE->GDT[2] = 0x0000920000000000ULL; // Data Segment
	GDTR64 gdtr;
	gdtr.L = 0x17;
	gdtr.A = (QWORD) SYSTEM_TABLE->GDT;
	__lgdt(&gdtr);
	((void(*)(void *)) LXS0)(LXS1);
}