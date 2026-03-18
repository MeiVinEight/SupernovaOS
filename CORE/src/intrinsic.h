#pragma once

#include <types.h>

#pragma section(".text")
#define COREAPI __declspec(allocate(".text"))

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