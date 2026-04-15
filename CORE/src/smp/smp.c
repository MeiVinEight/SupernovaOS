#include <smp/smp.h>
#include <interrupt/apic.h>
#include <stdio.h>

void int_smp(INTERRUPT_STACK *stack)
{
	printf("CPU #%lu INT: %02llX @ RIP %p: Multiprocessor\n", apic_current_id(), stack->INT, (void *) stack->RIP);
	//eoi_apic(0);
}