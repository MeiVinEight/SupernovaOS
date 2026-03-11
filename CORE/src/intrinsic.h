#pragma once

#pragma section(".text")
#define COREAPI __declspec(allocate(".text"))


typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long long QWORD;

typedef struct _MEMORY_REGION
{
	QWORD A;
	QWORD L : 56;
	QWORD F : 8;
} MEMORY_REGION;
typedef struct _SUPERNOVA_SYSTEM_TABLE
{
	QWORD GUID0[2];
	QWORD GUID1[2];
	WORD HRES;
	WORD VRES;
	DWORD PPL;
	QWORD FBB;
	QWORD GDT[3];
	MEMORY_REGION MEMORY[1];
} SUPERNOVA_SYSTEM_TABLE;

void  __cpuid(int *, int);
void  __cpuidex(int *, int, int);
void  __halt();
BYTE  __inbyte(DWORD);
DWORD __indword(DWORD);
WORD  __inword(DWORD);
void  __lidt(void *);
void  __nop();
void  __outbyte(DWORD, BYTE);
void  __outdword(DWORD, DWORD);
void  __outword(DWORD, WORD);
QWORD __rdtsc();
QWORD __readcr0(void);
QWORD __readcr2(void);
QWORD __readcr3(void);
QWORD __readcr4(void);
QWORD __readmsr(DWORD);
WORD  __reverse16(WORD);
DWORD __reverse32(DWORD);
QWORD __reverse64(QWORD);
void  __sidt(void *);
void  __writecr0(QWORD);
void  __writecr3(QWORD);
void  __writecr4(QWORD);
void  __writemsr(DWORD, QWORD);

QWORD __lgdt();
QWORD __getrsp();
void *__memset128(void *, void *, unsigned long long);

extern SUPERNOVA_SYSTEM_TABLE *SYSTEM_TABLE;