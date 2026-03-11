#include <intrinsic.h>
#include <console.h>
#include <interrupt/interrupt.h>
#include <memory/segment.h>
#include <memory/paging.h>
#include <arch/processor.h>
#include <interrupt/apic.h>
#include <timer/timer.h>

#include "driver/pci/pci.h"

COREAPI SUPERNOVA_SYSTEM_TABLE *SYSTEM_TABLE = (SUPERNOVA_SYSTEM_TABLE *) 0x0040;


unsigned long long _DllMainCRTStartup()
{
	setup_paging();

	QWORD buf[2] = { 0, 0 };
	__memset128((void *) SYSTEM_TABLE->FBB, buf, (SYSTEM_TABLE->PPL * SYSTEM_TABLE->VRES * 4) / 16);
	simple_output("Supernova OS\n");

	setup_processor();
	setup_segment();
	setup_apic();
	setup_interrupe();

	DWORD cpuid[4];
	char brand[51];
	brand[48] = '\r';
	brand[49] = '\n';
	brand[50] = 0;
	DWORD *brandx = (DWORD *) brand;
	__cpuid((int *) cpuid, (int) 0x80000002);
	brandx[0] = cpuid[0];
	brandx[1] = cpuid[1];
	brandx[2] = cpuid[2];
	brandx[3] = cpuid[3];
	__cpuid((int *) cpuid, (int) 0x80000003);
	brandx[4] = cpuid[0];
	brandx[5] = cpuid[1];
	brandx[6] = cpuid[2];
	brandx[7] = cpuid[3];
	__cpuid((int *) cpuid, (int) 0x80000004);
	brandx[8] = cpuid[0];
	brandx[9] = cpuid[1];
	brandx[10] = cpuid[2];
	brandx[11] = cpuid[3];
	simple_output("CPU #");
	simple_output_number(apic_current_id());
	outchar(' ');
	simple_output(brand);
	outchar('\n');

	setup_timer();
	setup_pci();


	MEMORY_REGION *beg = SYSTEM_TABLE->MEMORY;
	//             0000000000000000 | 0000000000000000 | 00000000
	simple_output("Base Address       Length             Type\r\n");
	while (~beg->A)
	{
		simple_output_address(beg->A, 16);
		simple_output(" | ");
		simple_output_address(beg->L, 16);
		simple_output(" | ");
		simple_output_address(beg->F, 2);
		simple_output("\r\n");
		beg++;
	}

	simple_output("GDT: ");
	simple_output_address((QWORD) SYSTEM_TABLE->GDT, 16);
	simple_output("\n");

	simple_output("OK\n");

	*((DWORD *) 0x100000000ULL) = 1;

	while (1) __halt();
}