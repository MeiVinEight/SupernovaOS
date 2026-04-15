#include <memory/paging.h>
#include <intrinsic.h>
#include <interrupt/interrupt.h>
#include <arch/processor.h>
#include <memory/virtmem.h>
#include <stdio.h>

void INT0E(INTERRUPT_STACK *stack)
{
	QWORD addr = __readcr2();
	if (addr >= 0xFFFF800000000000ULL)
	{
		if (addr < 0xFFFF808000000000ULL)
		{
			QWORD phyAddr = addr & 0x0000007FFFE00000ULL;
			addr &= ~((1ULL << 21) - 1);
			virtual_mapping(phyAddr, addr, 1, PAGE_2M, PA_WRITE);
			return;
		}
	}

	printf("CPU #%u INT: #PF @ RIP %p\nCODE: %p\nADDR: %p\n", cpu_local_apic_id(), (void *) stack->RIP, (void *) stack->ERROR, (void *) __readcr2());
	while (1) __halt();
}
void setup_paging()
{
	// Setup Paging
	QWORD *paging = (QWORD *) 0x1000;
	paging[0x000] = 0x2003;
	paging[0x100] = 0x2003;
	paging = (QWORD *) 0x2000;
	paging[0] = 0x3003;
	paging[1] = 0x4003;
	paging[2] = 0x5003;
	paging[3] = 0x6003;
	QWORD memAddr = 0;
	for (QWORD i = 0; i < 4; i++)
	{
		paging = (QWORD *) (0x3000 + (i * 0x1000));
		for (QWORD j = 0; j < 512; j++)
		{
			paging[j] = memAddr | 0x183;
			memAddr += 0x00200000;
		}
	}
	__writecr3(0x1000);
}
void setup_page_fault()
{
	register_interrupt(0x0E, INT0E);
}