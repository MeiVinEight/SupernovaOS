#ifndef __INTRINSIC_H__
#define __INTRINSIC_H__

//typedef QWORD size_t;

//#include <immintrin.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long long QWORD;

void __halt();
void __cpuid(void *, DWORD);
QWORD __readcr0(void);
void __writecr0(QWORD);
QWORD __readcr3(void);
QWORD __readcr4(void);
void __writecr4(QWORD);
void __lidt(void *);
void *memset(void *, int, unsigned long long);
void *memset64(void *, QWORD, unsigned long long);
void *__cdecl memcpy(void *, const void *, unsigned long long);
void __cli();
void __setrsp(unsigned long long);
QWORD __lgdt();
void __outdword(unsigned short, unsigned long);
void __outword(unsigned short, unsigned short);
unsigned long __indword(unsigned short);
unsigned short __inword(unsigned short);
void *__memset128(void *dst, void *, unsigned long long);

/*
void *memset128(void *dst, __m128i val, unsigned long long cnt)
{
	char *dstx = (char *) dst;
	while (cnt--)
	{
		_mm_store_si128((__m128i *) dstx, val);
		dstx += 16;
	}
	return dst;
}
*/

#endif