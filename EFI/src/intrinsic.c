#pragma section(".text")

__declspec(allocate(".text")) char __memset[] =
/*
void *__cdecl memset(void *, int, unsigned long long)
*/
{
	0x57,             // PUSH   RDI
	0x48, 0x8B, 0xF9, // MOV    RDI, RCX
	0x48, 0x63, 0xC2, // MOVSXD RAX, EDX
	0x49, 0x8B, 0xC8, // MOV    RCX, R8
	0x4C, 0x8B, 0xC7, // MOV    R8,  RDI
	0xF3, 0xAA,       // REP STOSB
	0x49, 0x8B, 0xC0, // MOV    RAX, R8
	0x5F,             // POP    RDI
	0xC3,             // RETN
};
__declspec(allocate(".text")) char memset64[] =
/*
void *__cdecl memset(void *, int, unsigned long long)
*/
{
	0x51,             // PUSH   RCX
	0x52,             // PUSH   RDX
	0x41, 0x50,       // PUSH   R8
	0x57,             // PUSH   RDI
	0x48, 0x8B, 0xF9, // MOV    RDI, RCX
	0x48, 0x8B, 0xC2, // MOV    RAX, RDX
	0x49, 0x8B, 0xC8, // MOV    RCX, R8
	0x4C, 0x8B, 0xC7, // MOV    R8,  RDI
	0xF3, 0x48, 0xAB, // REP STOSQ
	0x5F,             // POP    RDI
	0x41, 0x58,       // POP    R8
	0x5A,             // POP    RDX
	0x59,             // POP    RCX
	0x48, 0x8B, 0xC1, // MOV    RAX, RCX
	0xC3,             // RETN
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
__declspec(allocate(".text")) char __cli[] =
{
	0xFA, // CLI
	0xC3, // RET
};
__declspec(allocate(".text")) char __setrsp[] =
{
	0x58,             // POP RAX
	0x48, 0x8B, 0xE1, // MOV RSP, RCX
	0x48, 0xFF, 0xE0, // JMP RAX
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
int __memcmp(const void *a, const void *b, unsigned long long size)
{
	char *bufA = (char *) a;
	char *bufB = (char *) b;
	while (size--)
	{
		if (*bufA != *bufB)
			return (unsigned char) *bufA - (unsigned char) *bufB;
		bufA++;
		bufB++;
	}
	return 0;
}