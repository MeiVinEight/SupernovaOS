#include <interrupt/interrupt.h>
#include <console.h>
#include <core.h>
#include <intrinsic.h>
#include <interrupt/apic.h>
#include <stdio.h>
#include <arch/tss.h>
#include <mm/vmm.h>

COREAPI volatile INTERRUPT64 * volatile IDT;
COREAPI void (*(INTERRUPT_ROUTINE[256]))(INTERRUPT_STACK *);
COREAPI BYTE ISR[256][9];
COREAPI BYTE __isr[] =
{
	0x50,                         // PUSH RAX
	0x51,                         // PUSH RCX
	0x52,                         // PUSH RDX
	0x53,                         // PUSH RBX
	0x54,                         // PUSH RSP
	0x55,                         // PUSH RBP
	0x56,                         // PUSH RSI
	0x57,                         // PUSH RDI
	0x41, 0x50,                   // PUSH R8
	0x41, 0x51,                   // PUSH R9
	0x41, 0x52,                   // PUSH R10
	0x41, 0x53,                   // PUSH R11
	0x41, 0x54,                   // PUSH R12
	0x41, 0x55,                   // PUSH R13
	0x41, 0x56,                   // PUSH R14
	0x41, 0x57,                   // PUSH R15
	0x48, 0x8B, 0xCC,             // MOV RCX, RSP
	0x48, 0x83, 0xEC, 0x10,       // SUB RSP, 10H
	0xE8, 0x00, 0x00, 0x00, 0x00, // CALL 00000000
	0x48, 0x83, 0xC4, 0x10,       // ADD RSP, 10H
	0x41, 0x5F,                   // POP R15
	0x41, 0x5E,                   // POP R14
	0x41, 0x5D,                   // POP R13
	0x41, 0x5C,                   // POP R12
	0x41, 0x5B,                   // POP R11
	0x41, 0x5A,                   // POP R10
	0x41, 0x59,                   // POP R9
	0x41, 0x58,                   // POP R8
	0x5F,                         // POP RDI
	0x5E,                         // POP RSI
	0x5D,                         // POP RBP
	0x5C,                         // POP RSP
	0x5B,                         // POP RBX
	0x5A,                         // POP RDX
	0x59,                         // POP RCX
	0x58,                         // POP RAX
	0x48, 0x83, 0xC4, 0x10,       // ADD RSP, 10
	0x48, 0xCF,                   // IRETQ
};
COREAPI volatile DWORD INTE_VECTOR[32];

void __stdcall __isr_common(INTERRUPT_STACK *stack)
{
	BYTE id = stack->INT;
	if (INTERRUPT_ROUTINE[id])
	{
		QWORD rsp = stack->RSP;
		stack->RSP = ((QWORD) stack) + sizeof(INTERRUPT_STACK) - 0x10;
		if (stack->CS & 3)
			stack->RSP = stack->STACK;
		INTERRUPT_ROUTINE[id](stack);
		stack->RSP = rsp;
		return;
	}

	panic(stack);
}
void __stdcall setup_interrupt()
{
	IDT = (INTERRUPT64 *) SYSTEM_TABLE->IDT;
	DWORD erc = 0x60227D00; // ERROR CODE Mask
	// Calculate RVA from __isr+0x1B to __isr_common
	QWORD rva = (QWORD) (__isr + 0x20);
	*((DWORD *) rva) = (DWORD) (((QWORD) __isr_common) - (rva + 4));

	for (DWORD i = 0; i < 256; i++)
	{
		// Setup Interrupt common
		BYTE *isrx = ISR[i];
		if ((i < 32) && ((erc >> i) & 1))
		{
			// Have error code
			// NOP
			isrx[0] = 0x90;
			// NOP
			isrx[1] = 0x90;
		}
		else
		{
			// No error code
			// PUSH 0x00
			isrx[0] = 0x6A;
			isrx[1] = 0x00;
		}
		// PUSH id
		isrx[2] = 0x6A;
		isrx[3] = i;
		// JMP __isr
		isrx[4] = 0xE9;
		// RVA __isr
		rva = (QWORD) (isrx + 5);
		*((DWORD *) rva) = (DWORD) (((QWORD) __isr) - (rva + 4));
		QWORD isr0a = (QWORD) isrx;

		IDT[i].ADR0 = (isr0a >> 0x00) & 0xFFFF;
		IDT[i].SEGS = 0x08; // Code Segment
		IDT[i].TYPE = 0x0E; // Interrupt Gate
		IDT[i].PRST = 1;
		IDT[i].ADR1 = (isr0a >> 0x10) & 0xFFFF;
		IDT[i].ADR2 = (isr0a >> 0x20);
	}
	// Load IDT
	IDTR64 idtr;
	idtr.Limit = 0xFFF;
	*((QWORD *) &idtr.Base) = (QWORD) IDT;
	__lidt(&idtr);

	// Setup Interrupt Controller
	// APIC


	// Reserve int ids
	for (int i = 0; i < 6; i++)
		INTE_VECTOR[i] = 0xFF;
	interrupt_set_intx(0x80); // Reserved for syscall

	// Enable Interrupt
	__sti();
}
void register_interrupt(BYTE id, void (*routine)(INTERRUPT_STACK *))
{
	INTERRUPT_ROUTINE[id] = routine;
}
void interrupt_set_intx(BYTE id)
{
	INTE_VECTOR[id >> 3] |= (1 << (id & 7));
}
void interrupt_free_intx(BYTE id)
{
	INTE_VECTOR[id >> 3] &= ~(1 << (id & 7));
}
BYTE interrupt_alloc_intx()
{
	for (int id = 0; id < 256; id++)
	{
		if (!(INTE_VECTOR[id >> 3] & (1 << (id & 7))))
		{
			interrupt_set_intx(id);
			return id;
		}
	}
	return 0;
}