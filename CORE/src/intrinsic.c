//#include <intrinsic.h>
#pragma section(".text")

__declspec(allocate(".text")) char __getrsp[] = {
	0x48, 0x8D, 0x44, 0x24, 0x08, // LEA RAX, [RSP+08H];
	0xC3,                         // RET
};
__declspec(allocate(".text")) char __memset128[] =
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
__declspec(allocate(".text")) char __memcpy128[] = {
	0x48, 0x8B, 0xC1,       // MOV RAX, RCX

	// LOOP0:
	0x4D, 0x85, 0xC0,       // TEST R8, R8
	0x74, 0x15,             // JZ LOOP1

	0xF3, 0x0F, 0x6F, 0x02, // MOV
	0xF3, 0x0F, 0x7F, 0x01, // MOV
	0x48, 0x83, 0xC1, 0x10, // ADD RCX, 10H
	0x48, 0x83, 0xC2, 0x10, // ADD RDX, 10H
	0x49, 0xFF, 0xC8,       // DEC R8
	0xEB, 0xE6,             // JMP LOOP0

	// LOOP1:
	0xC3,                   // RET
};
__declspec(allocate(".text")) char __lgdt[] =
{
	0x0F, 0x01, 0x11, // LGDT [RCX]
	0xC3              // RET
};
__declspec(allocate(".text")) char __sti[] =
{
	0xFB, // STI
	0xC3, // RET
};
__declspec(allocate(".text")) char __memcpy[] =
/*
void *__cdecl memcpy(void *, const void *, unsigned long long)
*/
{
	0x56,             // PUSH RSI
	0x57,             // PUSH RDI
	0x48, 0x8B, 0xC1, // MOV  RAX, RCX
	0x48, 0x8B, 0xF9, // MOV  RDI, RCX
	0x48, 0x8B, 0xF2, // MOV  RSI, RDX
	0x49, 0x8B, 0xC8, // MOV  RCX, R8
	0xF3, 0xA4,       // REP MOVSB
	0x5F,             // POP  RDI
	0x5E,             // POP  RSI
	0xC3,             // RETN
};
__declspec(allocate(".text")) char __int[] =
/*
void __int(BYTE)
*/
{
	0x88, 0x0D, 0x01, 0x00, 0x00, 0x00, // MOV BYTE PTR [$+07H], CL
	0xCD, 0x00,                         // INT 00H
	0xC3,                               // RET
};
__declspec(allocate(".text")) char __setrbp[] =
/*
void __setrbp(QWORD)
*/
{
	0x48, 0x8B, 0xE9, // MOV RBP, RCX
	0xC3,             // RET
};
__declspec(allocate(".text")) char __memset[] =
/*
void *__memset(volatile void *, BYTE, QWORD);
*/
{
	0x48, 0x8B, 0xC1, // MOV RAX, RCX

	// LOOP1:
	0x4D, 0x85, 0xC0, // TEST R8, R8
	0x74, 0x0A,       // JZ LOOP2

	0x88, 0x11,       // MOV BYTE PTR [RCX], DL
	0x48, 0xFF, 0xC1, // INC RCX
	0x49, 0xFF, 0xC8, // DEC R8
	0xEB, 0xF1,       // JMP LOOP1

	// LOOP2:
	0xC3,             // RET
};
__declspec(allocate(".text")) char __ltr[] =
{
	0x0F, 0x00, 0xD9, // LTR CX
	0xC3,             // RET
};
__declspec(allocate(".text")) char __iret[] =
{
	0x41, 0x50, // PUSH R8
	0x41, 0x51, // PUSH R9
	0x9C,       // PUSHFQ
	0x51,       // PUSH RCX
	0x52,       // PUSH RDX
	0x48, 0xCF, // IRETQ
};
__declspec(allocate(".text")) char __syscall[] =
{
	0xCD, 0x2E, // INT 2EH
	0xC3,       // RET
};