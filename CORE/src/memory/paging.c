#include <memory/paging.h>
#include <intrinsic.h>
#include <console.h>

void INT0E(INTERRUPT_STACK *stack)
{
	simple_output("CPU #");
	simple_output_number(cpu_local_apic_id());
	simple_output(" INT: #PF @ RIP ");
	simple_output_address(stack->RIP, 16);
	simple_output("\n");
	simple_output("CODE: ");
	simple_output_address(stack->ERROR, 16);
	simple_output("\n");

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