#include <interrupt/apic.h>
#include <interrupt/8259a.h>
#include <arch/ia32_msr.h>
#include <intrinsic.h>
#include <console.h>
#include <core.h>
#include <timer/hpet.h>
#include <timer/rtc.h>
#include <interrupt/interrupt.h>
#include <arch/processor.h>


#define CPUID_FEAT_EDX_APIC (1 << 9)

COREAPI DWORD USEAPIC = 0;
COREAPI volatile DWORD(*volatile APIC_REGISTERS)[4];
COREAPI volatile QWORD CPU_MASK = 0;
COREAPI volatile DWORD APIC_BSP_LOCK = 0;
/*
COREAPI BYTE AP_BOOT_CODE[] =
{
	0xFA,                                           // CS:0000 CLI
	0xEA, 0x20, 0x00, 0x00, 0x00,                   // CS:0001 JMP FAR CS:0020
	0xEB, 0xFE,                                     // CS:0006 JMP $+00
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // CS:0008 DQ 0000000000000000
	0x00, 0x00, 0x00, 0x00, 0x00, 0x9A, 0x20, 0x00, // CS:0010 DQ 00209A0000000000
	0x00, 0x00, 0x00, 0x00, 0x00, 0x92, 0x00, 0x00, // CS:0018 DQ 0000920000000000
	0x8C, 0xC8,                                     // CS:0020 MOV AX, CS
	0x8E, 0xD8,                                     // CS:0022 MOV DS, AX
	0x8E, 0xC0,                                     // CS:0024 MOV ES, AX
	0x8E, 0xD0,                                     // CS:0026 MOV SS, AX
	0xBC, 0x00, 0x10,                               // CS:0028 MOV SP, 1000
	0x66, 0x33, 0xC0,                               // CS:002B XOR EAX, EAX
	0x8C, 0xC8,                                     // CS:002E MOV AX, CS
	0xB1, 0x04,                                     // CS:0030 MOV CL, 04
	0x66, 0xD3, 0xE0,                               // CS:0032 SHL EAX, CL
	0xBE, 0xA5, 0x00,                               // CS:0035 MOV SI, 00A5
	0x66, 0x01, 0x04,                               // CS:0038 ADD [SI], EAX
	0xE4, 0x92,                                     // CS:0x3B IN AL, 92
	0x0C, 0x02,                                     // CS:003D OR AL, 02
	0xE6, 0x92,                                     // CS:003F OUT 92, AL
	0x66, 0x33, 0xC0,                               // CS:0041 XOR EAX, EAX
	0x66, 0x50,                                     // CS:0044 PUSH EAX
	0x8C, 0xC8,                                     // CS:0046 MOV AX, CS
	0xB1, 0x0C,                                     // CS:0048 MOV CL, 0C
	0xD3, 0xE8,                                     // CS:004A SHR AX, CL
	0x50,                                           // CS:004C PUSH AX
	0x8C, 0xC8,                                     // CS:004D MOV AX, CS
	0xB1, 0x04,                                     // CS:004F MOV CL, 04
	0xD3, 0xE0,                                     // CS:0051 SHL AX, CL
	0xB9, 0x08, 0x00,                               // CS:0053 MOV CX, 0008
	0x03, 0xC1,                                     // CS:0056 ADD AX, CX
	0x50,                                           // CS:0058 PUSH AX
	0xB8, 0x17, 0x00,                               // CS:0059 MOV AX, 0017
	0x50,                                           // CS:005C PUSH AX
	0x8B, 0xF4,                                     // CS:005D MOV SI, SP
	0x3E, 0x0F, 0x01, 0x14,                         // CS:005F LGDT DS:[SI]
	0x83, 0xC4, 0x0A,                               // CS:0063 ADD SP, 000A
	0x66, 0xB8, 0x00, 0x10, 0x00, 0x00,             // CS:0066 MOV EAX, 00001000
	0x66, 0x0F, 0x22, 0x18,                         // CS:006C MOV CR3, EAX
	0x66, 0x0F, 0x20, 0x20,                         // CS:0070 MOV EAX, CR4
	0x66, 0x83, 0xC8, 0x20,                         // CS:0074 OR EAX, 20
	0x66, 0x0D, 0x80, 0x00, 0x00, 0x00,             // CS:0078 OR EAX, 00000080
	0x66, 0x0F, 0x22, 0x20,                         // CS:007E MOV CR4, EAX
	0x66, 0xB9, 0x80, 0x00, 0x00, 0xC0,             // CS:0082 MOV ECX, C0000080
	0x0F, 0x32,                                     // CS:0088 RDMSR
	0x66, 0x0D, 0x00, 0x01, 0x00, 0x00,             // CS:008A OR EAX, 00000100
	0x0F, 0x30,                                     // CS:0090 WRMSR
	0x66, 0x0F, 0x20, 0x00,                         // CS:0092 MOV EAX, CR0
	0x66, 0xB9, 0x01, 0x00, 0x00, 0x80,             // CS:0096 MOV ECX, 80000001
	0x66, 0x0B, 0xC1,                               // CS:009C OR EAX, ECX
	0x66, 0x0F, 0x22, 0x00,                         // CS:009F MOV CR0, EAX
	0x66, 0xEA, 0xAB, 0x00, 0x00, 0x00, 0x08, 0x00, // CS:00A3 JMP FAR 0008:000000AB
	                                                            // [Long-Mode]
	0x31, 0xC9,                                                 // 000000AB XOR ECX, ECX
	0x66, 0x8E, 0xE1,                                           // 000000AD MOV FS, CX
	0x66, 0x8E, 0xE9,                                           // 000000B0 MOV GS, CX
	0x83, 0xC1, 0x10,                                           // 000000B3 ADD ECX, 10
	0x66, 0x8E, 0xC1,                                           // 000000B6 MOV ES, CX
	0x66, 0x8E, 0xD1,                                           // 000000B9 MOV SS, CX
	0x66, 0x8E, 0xD9,                                           // 000000BC MOV DS, CX
	0x48, 0xBC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 000000BF MOV RSP, 0000000000000000
	0xB9, 0x00, 0x00, 0x00, 0x00,                               // 000000C9 MOV ECX, 00000000
	0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,                         // 000000CE JMP QWORD PTR [00000000]
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // 000000D4 DQ 0000000000000000
};
*/


void eoi_apic(BYTE id)
{
	APIC_REGISTERS[APIC_EOI][0] = 0;
}
int check_apic()
{
	int cpui[4] = { 0 };
	__cpuid(cpui, 1);
	return !!(cpui[3] & CPUID_FEAT_EDX_APIC);
}
void setup_apic()
{
	if (!check_apic())
	{
		// USE 8259A
		setup_8259A();
		return;
	}

	if (!USEAPIC)
	{
		USEAPIC = 1;
		disable_8259A();
	}

	QWORD apicBaseMsr = __readmsr(IA32_APIC_BASE_MSR);
	QWORD apicBase = apicBaseMsr & (~0xFFFULL);

	// Hardware Enable APIC
	__writemsr(IA32_APIC_BASE_MSR, apicBaseMsr |= IA32_APIC_BASE_MSR_ENABLE);

	if (!APIC_REGISTERS)
		APIC_REGISTERS = (DWORD (*)[4]) core_mapping(apicBase);


	// Set TPR to 0, receive all interrupts
	APIC_REGISTERS[APIC_TPR][0] = 0;
	// Set DFR all bits to 1 for use flat model
	APIC_REGISTERS[APIC_DFR][0] = 0xFFFFFFFF;

	APIC_REGISTERS[APIC_LDR][0] = (APIC_REGISTERS[APIC_LDR][0] & 0x00FFFFFF) | 1;

	APIC_REGISTERS[APIC_CMCI][0] = (1 << 17);
	// Clear all lvt
	APIC_REGISTERS[APIC_LVT0][0] = APIC_LVT_CLR;
	APIC_REGISTERS[APIC_LVT1][0] = (1 << 17);
	APIC_REGISTERS[APIC_LVT2][0] = APIC_LVT_NMI;
	APIC_REGISTERS[APIC_LVT3][0] = (1 << 17);
	APIC_REGISTERS[APIC_LVT4][0] = (1 << 17);

	// Software enable APIC
	// Set the Spurious Interrupt Vector Register bit 8 to start receiving interrupts
	APIC_REGISTERS[APIC_SIV][0] = APIC_SPURIOUS_VECTOR | APIC_SOFTWARE_ENABLE;
}
DWORD apic_current_id()
{
	int reg[4];
	__cpuid(reg, 1);
	if (APIC_REGISTERS)
	{
		return APIC_REGISTERS[APIC_APICID][0] >> 24;
	}
	return 0;
}
void setup_apic_timer(DWORD rate)
{
	// Map APIC timer to an interrupt, and by that enable it in one-shot mode
	APIC_REGISTERS[APIC_LVT0][0] = (IRQ_INT + APIC_TIMER_IRQ);
	// Set up divide value to 1
	// See Intel® 64 and IA-32 Architectures Software Developer’s Manual. Volume 2. Figure 11.10
	APIC_REGISTERS[APIC_TDCR][0] = APIC_TIMER_DCR_1;

	/*
	// Prepare PIT sleep 50 ms
	__outbyte(PIT2_GATE, (__inbyte(PIT2_GATE) & 0xFD) | 1);
	__outbyte(PIT_CMD, 0B10110010);
	DWORD frq = PIT_FREQUENCY / rate;
	__outbyte(PIT2_DATA, (frq >> 0) & 0xFF);
	__inbyte(0x60);
	__outbyte(PIT2_DATA, (frq >> 8) & 0xFF);

	BYTE al = __inbyte(PIT2_GATE) & 0xFE;
	// Reset PIT one-shot counter (start counting)
	__outbyte(PIT2_GATE, al);
	__outbyte(PIT2_GATE, al | 1);
	*/

	QWORD hpetHwFreq = hpet_query_frequency();
	if (hpetHwFreq)
	{
		simple_output("HPET ");
		simple_output_number(hpetHwFreq);
		simple_output(" Hz\n");
		QWORD startHpet = hpet_get_counter();
		// Reset APIC timer (set counter to 0xFFFFFFFF)
		APIC_REGISTERS[APIC_TICR][0] = 0xFFFFFFFFUL;

		// Now wait until PIT counter reaches zero
		// while (!(__inbyte(PIT2_GATE) & 0x20)) {}
		// Wait HPET for 1 second
		while (hpet_get_counter() - startHpet < hpetHwFreq) __nop();

		// Stop APIC timer
		//APIC_REGISTERS[APIC_TICR][0] = 0;
		APIC_REGISTERS[APIC_LVT0][0] = APIC_LVT_CLR;
	}
	else
	{
		// USE CMOS RTC Wait for 1 second
		BYTE rtcSec0 = rtc_get_second();
		BYTE rtcSec1;
		while ((rtcSec1 = rtc_get_second()) == rtcSec0) __nop();

		// Reset APIC timer (set counter to 0xFFFFFFFF)
		APIC_REGISTERS[APIC_TICR][0] = 0xFFFFFFFFUL;

		while (rtc_get_second() == rtcSec1) __nop();

		// Stop APIC timer
		//APIC_REGISTERS[APIC_TICR][0] = 0;
		APIC_REGISTERS[APIC_LVT0][0] = APIC_LVT_CLR;
	}


	QWORD apicTick = APIC_REGISTERS[APIC_TCCR][0];

	// Get APIC timer frequency
	QWORD freq = 0xFFFFFFFFU - apicTick;


	QWORD freqKHz = freq / 1000;
	QWORD shift = 1;
	while (freqKHz > 999)
	{
		freqKHz++;
		freqKHz /= 10;
		shift *= 10;
	}
	freqKHz *= shift;
	simple_output("APIC ");
	simple_output_number(freqKHz / 1000);
	simple_output(" MHz (");
	simple_output_number(freq);
	simple_output(" Hz)\n");

	// Use it as APIC timer counter initializer
	APIC_REGISTERS[APIC_TICR][0] = freq / rate;
	// Setting divide value register again not needed by the manuals
	APIC_REGISTERS[APIC_TDCR][0] = APIC_TIMER_DCR_1;
	// Finally re-enable timer in periodic mode
	APIC_REGISTERS[APIC_LVT0][0] = (IRQ_INT + APIC_TIMER_IRQ) | APIC_TIMER_MODE_PERIODIC;
}
void apic_write_interrupt_command(QWORD x)
{
	while (APIC_REGISTERS[APIC_ICRL][0] & APIC_ICR_DELIVERY_STATUS);
	APIC_REGISTERS[APIC_ICRH][0] = (x >> 32) & 0xFFFFFFFF;
	APIC_REGISTERS[APIC_ICRL][0] = (x >>  0) & 0xFFFFFFFF;
	while (APIC_REGISTERS[APIC_ICRL][0] & APIC_ICR_DELIVERY_STATUS);
}
void apic_ipi(BYTE apicId, BYTE intr)
{
	apic_write_interrupt_command((((QWORD) apicId) << 56) | APIC_ICR_LEVEL_ASSERT | intr);
}
void apic_startup_ap(BYTE apicid, void (*apEntry)(void))
{
	if (apicid == apic_current_id())
		return;

	*((QWORD *) (SYSTEM_TABLE->APC + 0x95)) = (QWORD) apEntry;

	APIC_BSP_LOCK = 1;

	// INIT
	apic_write_interrupt_command(APIC_ICR_LEVEL_ASSERT | APIC_ICR_DELIVERY_INIT | (((QWORD) apicid) << 56));
	// Delay
	__halt();
	// SIPI
	apic_write_interrupt_command(APIC_ICR_LEVEL_ASSERT | APIC_ICR_DELIVERY_STARTUP | /*((((QWORD) SYSTEM_TABLE->APC) >> 12) & 0xFF)*/9 | (((QWORD) apicid) << 56));
	while (APIC_BSP_LOCK) __halt();


	/*
	// Check BSP
	APIC_REGISTERS[APIC_ICRH][0] = APIC_REGISTERS[APIC_ICRH][0] | (((QWORD) apicid) << 24); // select AP
	APIC_REGISTERS[APIC_ICRL][0] = (APIC_REGISTERS[APIC_ICRL][0] & 0xFFF00000) | 0x0000C500; // trigger INIT IPI
	while (APIC_REGISTERS[APIC_ICRL][0] & APIC_ICR_DELIVERY_STATUS) __nop(); // wait for delivery
	APIC_REGISTERS[APIC_ICRH][0] = APIC_REGISTERS[APIC_ICRH][0] | (((QWORD) apicid) << 24); // select AP
	APIC_REGISTERS[APIC_ICRL][0] = (APIC_REGISTERS[APIC_ICRL][0] & 0xFFF00000) | 0x00008500; // deassert
	while (APIC_REGISTERS[APIC_ICRL][0] & APIC_ICR_DELIVERY_STATUS) __nop(); // wait for delivery
	__halt();
	// send STARTUP IPI (twice)
	for (int j = 0; j < 2; j++)
	{
		APIC_REGISTERS[APIC_ESR][0] = 0; // clear APIC errors
		APIC_REGISTERS[APIC_ICRH][0] = APIC_REGISTERS[APIC_ICRH][0] | (((QWORD) apicid) << 24); // select AP
		APIC_REGISTERS[APIC_ICRL][0] = (APIC_REGISTERS[APIC_ICRL][0] & 0xFFF0F800) | 0x00000609; // trigger STARTUP IPI for 0900:0000
		// wait 200 usec
		__halt();
		__halt();
		__halt();
		__halt();
		while (APIC_REGISTERS[APIC_ICRL][0] & APIC_ICR_DELIVERY_STATUS) __nop(); // wait for delivery
	}
	*/

}
void setup_madt(ACPI_MADT *madt)
{
	/*
	QWORD apPhyAddr = ((QWORD) SYSTEM_TABLE->APC) & 0xFFFFF;
	__memcpy(SYSTEM_TABLE->APC, AP_BOOT_CODE, sizeof(AP_BOOT_CODE));
	*((WORD *) (SYSTEM_TABLE->APC + 4)) = apPhyAddr >> 4;
	QWORD apStack = 0xFFFF80000000A000ULL;
	*((QWORD *) (SYSTEM_TABLE->APC + 0xC1)) = apStack;
	*((QWORD *) (SYSTEM_TABLE->APC + 0xD4)) = (QWORD) apic_ap_startup;
	*/

	QWORD size = madt->HEAD.LENG - sizeof(ACPI_MADT);
	DWORD bsp = apic_current_id();
	BYTE *data = madt->DATA;
	QWORD read = 0;
	while (read < size)
	{
		APIC_MADT_LAPIC *lapic = (APIC_MADT_LAPIC *) data;
		if (!lapic->TYP) // Processor Local APIC
		{
			DWORD apicId = lapic->AID;
			if ((lapic->FLG & 1) && (apicId != bsp))
				CPU_MASK |= (1ULL << apicId);
		}
		// else // I/O APIC
		read += lapic->SZE;
		data += lapic->SZE;
	}
}
void __stdcall aproc_startup(void)
{
	IDTR64 idtr;
	idtr.Limit = 0xFFF;
	*((QWORD *) &idtr.Base) = (QWORD) IDT;
	__lidt(&idtr);
	__sti();

	setup_processor();
	setup_apic();

	kprint_cpu();

	APIC_BSP_LOCK = 0;
	while (1) __halt();
}
void apic_setup_multiprocessor()
{
	for (int i = 0; i < 64; i++)
		if (CPU_MASK & (1ULL << i))
			apic_startup_ap(i, aproc_startup);
}