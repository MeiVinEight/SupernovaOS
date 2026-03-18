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
COREAPI DWORD(*APIC_REGISTERS)[4];
COREAPI volatile DWORD(*volatile APIC_REGISTERS)[4];


void eoi_apic(BYTE id)
{
	if (id >= IRQ_INT && id < (IRQ_INT + 0x10))
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