#include <interrupt/apic.h>
#include <interrupt/8259a.h>
#include <arch/ia32_msr.h>
#include <intrinsic.h>
#include <console.h>
#include <timer/8254.h>


#define CPUID_FEAT_EDX_APIC (1 << 9)

COREAPI DWORD USEAPIC = 0;
COREAPI DWORD(*APIC_REGISTERS)[4];


void eoi_apic(BYTE id)
{
	if (id >= IRQ_INT && id < (IRQ_INT + 0x10))
		APIC_REGISTERS[APIC_EOI][0] = 0;
}
int check_apic()
{
	DWORD cpui[4] = { 0 };
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

	USEAPIC = 1;
	disable_8259A();
	QWORD apicBaseMsr = __readmsr(IA32_APIC_BASE_MSR);
	QWORD apicBase = apicBaseMsr & (~0xFFFULL);

	// Hardware Enable APIC
	__writemsr(IA32_APIC_BASE_MSR, apicBaseMsr |= IA32_APIC_BASE_MSR_ENABLE);

	APIC_REGISTERS = (DWORD (*)[4]) apicBase;
	simple_output("APIC: ");
	simple_output_address(apicBase, 16);
	simple_output("\n");


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
	DWORD reg[4];
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

	simple_output("SETUP APIC TIMER ");
	simple_output_number(rate);
	simple_output(" Hz\n");

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
	// Reset APIC timer (set counter to 0xFFFFFFFF)
	APIC_REGISTERS[APIC_TICR][0] = 0xFFFFFFFFUL;

	// Now wait until PIT counter reaches zero
	while (!(__inbyte(PIT2_GATE) & 0x20)) {}

	// Stop APIC timer
	APIC_REGISTERS[APIC_LVT0][0] = APIC_LVT_CLR;

	// Get APIC timer frequency
	DWORD freq = (-(APIC_REGISTERS[APIC_TCCR][0])) + 1;
	freq *= rate;
	QWORD shift = 1;
	while (freq >= 99)
	{
		freq++;
		freq /= 10;
		shift *= 10;
	}
	freq *= shift;

	simple_output("APIC TIMER FREQUENCY ");
	simple_output_number(freq / 1000000);
	simple_output(" MHz\n");

	// Use it as APIC timer counter initializer
	APIC_REGISTERS[APIC_TICR][0] = freq / rate;
	// Setting divide value register again not needed by the manuals
	APIC_REGISTERS[APIC_TDCR][0] = APIC_TIMER_DCR_1;
	// Finally re-enable timer in periodic mode
	APIC_REGISTERS[APIC_LVT0][0] = (IRQ_INT + APIC_TIMER_IRQ) | APIC_TIMER_MODE_PERIODIC;
}