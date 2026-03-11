//#include <intrinsic.h>
#pragma section(".text")

__declspec(allocate(".text")) const char __getrsp[] = {
	0x48, 0x8D, 0x44, 0x24, 0x08, // LEA RAX, [RSP+08H];
	0xC3,                         // RET
};
__declspec(allocate(".text")) const char __memset128[] =
{
	0x48, 0x8B, 0xC1,       // MOV    RAX,  RCX
	0xF3, 0x0F, 0x6F, 0x02, // MOVDQU XMM0, XMMWORD PTR [RDX]

	// LOOP0:
	0x4D, 0x85, 0xC0,       // TEST   R8,   R8
	0x74, 0x0D,             // JZ     LOOP1

	0xF3, 0x0F, 0x7F, 0x01, // MOVDQU XMMWORD PTR [RCX], XMM0
	0x48, 0x83, 0xC1, 0x10, // ADD    RCX,  10H
	0x49, 0xFF, 0xC8,       // DEC    R8,
	0xEB, 0xEE,             // JMP    LOOP0

	// LOOP1:
	0xC3,                   // RET
};

__declspec(allocate(".text")) char __lgdt[] =
{
	0x0F, 0x01, 0x11, // LGDT [RCX]
	0xC3              // RET
};