#include <memory/segment.h>
#include <core.h>

COREAPI BYTE LXS1[] =
{
	0x48, 0x33, 0xC0,       // XOR RAX, RAX
	0x66, 0x8E, 0xC0,       // MOV ES, AX
	0x66, 0x8E, 0xD0,       // MOV SS, AX
	0x66, 0x8E, 0xD8,       // MOV DS, AX
	0x66, 0x8E, 0xE0,       // MOV FS, AX
	0x66, 0x8E, 0xE8,       // MOV GS, AX
	0x58,                   // POP RAX
	0x48, 0x0B, 0xE1,       // OR RSP, RCX
	0x6A, 0x08,             // PUSH 08H
	0x50,                   // PUSH RAX
	0x48, 0xCB              // RETFQ

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
	__lgdt((void *) core_mapping((QWORD) &gdtr));
	((void(*)(QWORD)) LXS1)(SYSTEM_ADDRESS);
}