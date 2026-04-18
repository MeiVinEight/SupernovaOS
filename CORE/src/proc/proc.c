#include <proc/proc.h>
#include <mm/vmm.h>
#include <intrinsic.h>
#include <stdio.h>
#include <mm/pmm.h>

extern BYTE __ImageBase;

PROCESS_CONTROL_BLOCK *volatile CURRENT_PROCESS;

PROCESS_CONTROL_BLOCK *create_process()
{
	PROCESS_CONTROL_BLOCK *pcb = heap_alloc(sizeof(PROCESS_CONTROL_BLOCK));
	__memset(pcb, 0, sizeof(PROCESS_CONTROL_BLOCK));
	vmm_free(&pcb->VMMA, 0, 0x800000000ULL);
	QWORD addr = 0x00007FFF00000000ULL;
	QWORD count = 24;
	virtual_alloc((physical_address((QWORD) pcb)), &addr, count, VMM_TYPE_RESERVE, VMM_WRITE | VMM_EXECUTE);
	LINEAR_MEMORY_BLOCK *pmm = pcb->VMMA;
	while (pmm)
	{
		printf("Process VMM: %p -> %p+%p\n", pmm, (void *) pmm->ADDR, (void *) pmm->SIZE);
		pmm = pmm->NEXT;
	}
	return pcb;
}
PROCESS_CONTROL_BLOCK *current_process()
{
	return CURRENT_PROCESS;
}