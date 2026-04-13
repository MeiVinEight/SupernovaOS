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
BYTE  _BitScanReverse(unsigned long * _Index, unsigned long _Mask);
long  _InterlockedCompareExchange(long volatile * Destination, long Exchange, long Comparand);
short _InterlockedCompareExchange16(short volatile * Destination, short Exchange, short Comparand);
void  _mm_pause(void);

QWORD __lgdt(void *);
void __ltr(WORD);
QWORD __getrsp();
void __setrbp(QWORD);
void *__memset128(void *, void *, unsigned long long);
void *__memcpy128(void *, void *, unsigned long long);
void __sti();
void *__memcpy(volatile void *, const void *, QWORD);
void *__memset(volatile void *, BYTE, QWORD);
void __stdcall __int(BYTE x);
void __iret(QWORD, QWORD, QWORD, QWORD);