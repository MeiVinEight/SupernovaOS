#include <timer/timer.h>
#include <interrupt/apic.h>
#include <console.h>
#include <interrupt/interrupt.h>
#include <intrinsic.h>
#include <stdio.h>

COREAPI volatile QWORD TSC_FREQUENCY_KHZ = 0;

void interrupt_timer(INTERRUPT_STACK *stack)
{
	// Do nothing
	eoi_apic(0);
}
void setup_timer()
{
	// APIC timer is set up in apic.c
	if (!USEAPIC)
	{
		printf("TIMER: APIC OFFLINE\n");
		return;
	}

	// Register IRQ0 interrupt
	register_interrupt(IRQ_INT, interrupt_timer);
	setup_apic_timer(TIMER_INT_FREQUENCY);

	DWORD reg[4];
	__cpuid((int *) reg, 1);
	if (!(reg[3] & (1 << 4)))
		return;

	QWORD start = __rdtsc();
	DWORD delay = TIMER_INT_FREQUENCY;
	while (delay--) __halt();
	QWORD end = __rdtsc();

	// Store calibrated CPU KHz
	TSC_FREQUENCY_KHZ = (end - start) / 1000;
	QWORD shift = 1;
	while (TSC_FREQUENCY_KHZ >= 99)
	{
		TSC_FREQUENCY_KHZ++;
		TSC_FREQUENCY_KHZ /= 10;
		shift *= 10;
	}
	TSC_FREQUENCY_KHZ *= shift;
	printf("CPU %llu MHz\n", TSC_FREQUENCY_KHZ / 1000);
}
void delay(QWORD ms)
{
	QWORD tsc = __rdtsc();
	tsc += TSC_FREQUENCY_KHZ * ms;
	__pause(tsc);
}