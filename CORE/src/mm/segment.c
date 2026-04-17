#include <mm/segment.h>
#include <core.h>
#include <arch/tss.h>

COREAPI BYTE LXS1[] =
{
	0x48, 0x33, 0xC0,       // XOR RAX, RAX
	0x66, 0x8E, 0xC0,       // MOV ES, AX
	0x66, 0x8E, 0xD0,       // MOV SS, AX
	0x66, 0x8E, 0xD8,       // MOV DS, AX
	0x66, 0x8E, 0xE0,       // MOV FS, AX
	0x66, 0x8E, 0xE8,       // MOV GS, AX
	0x58,                   // POP RAX
	0x6A, 0x08,             // PUSH 08H
	0x50,                   // PUSH RAX
	0x48, 0xCB              // RETFQ

};

void setup_gdt()
{
	// Setup GDT
	SYSTEM_TABLE->GDT[0] = 0x0000000000000000ULL; // NULL
	SYSTEM_TABLE->GDT[1] = 0x00209A0000000000ULL; // CODE R0
	SYSTEM_TABLE->GDT[2] = 0x0020FA0000000000ULL; // CODE R3
	SYSTEM_TABLE->GDT[3] = 0x0000F20000000000ULL; // DATA R3
}
void setup_segment()
{
	GDTR64 gdtr;
	gdtr.L = sizeof(SYSTEM_TABLE->GDT) - 1;
	gdtr.A = (QWORD) SYSTEM_TABLE->GDT;
	__lgdt(&gdtr);
	((void(*)()) LXS1)();
	load_tss((void *) SYSTEM_TABLE->TSS, (void *) SYSTEM_TABLE->GDT, 0x20);
}