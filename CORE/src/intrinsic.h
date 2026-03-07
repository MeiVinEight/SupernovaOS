#pragma once

#pragma section(".text")
#define COREAPI __declspec(allocate(".text"))


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
void __cli();
QWORD __lgdt();

void *__memset128(void *, void *, unsigned long long);