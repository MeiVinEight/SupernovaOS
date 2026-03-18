#pragma once

#include <types.h>

#define IRQ_INT 0x20

typedef struct _INTERRUPT64
{
	WORD  ADR0;     // Address [0:15]
	WORD  SEGS;     // Segment Selector
	BYTE  ISTB : 3; // Interrupt Stack Table
	BYTE  RSV0 : 5; // 0
	BYTE  TYPE : 4; // See Table 3-2. System-Segment and Gate-Descriptor Types
	BYTE  RSV1 : 1; // 0
	BYTE  PRVL : 2; // Privilege
	BYTE  PRST : 1; // Present
	WORD  ADR1;     // Address [16:31]
	DWORD ADR2;     // Address [32:63]
	DWORD RSV2;     // 0
} INTERRUPT64;
typedef struct _IDTR64
{
	WORD Limit;
	WORD Base[4];
} IDTR64;
typedef struct _INTERRUPT_STACK
{
	// General-purpose registers
	QWORD R15;
	QWORD R14;
	QWORD R13;
	QWORD R12;
	QWORD R11;
	QWORD R10;
	QWORD R9;
	QWORD R8;
	QWORD RDI;
	QWORD RSI;
	QWORD RBP;
	QWORD RSP;
	QWORD RBX;
	QWORD RDX;
	QWORD RCX;
	QWORD RAX;
	// Interrupt code
	QWORD INT;
	// Error code
	QWORD ERROR;
	// Interrupt auto push, RIP, CS, EFLAGS
	QWORD RIP;
	QWORD CS;
	QWORD EFLAGS;
} INTERRUPT_STACK;

extern volatile INTERRUPT64 * volatile IDT;

void setup_interrupe();
void register_interrupt(BYTE id, void (*routine)(INTERRUPT_STACK *));