#include <core.h>
#include <intrinsic.h>
#include <console.h>
#include <interrupt/interrupt.h>
#include <memory/segment.h>
#include <memory/paging.h>
#include <arch/processor.h>
#include <interrupt/apic.h>
#include <timer/timer.h>
#include <acpi/acpi.h>
#include <driver/pci/pcie.h>
#include <memory/virtmem.h>

QWORD __stdcall coreCRTStartup();

COREAPI volatile SUPERNOVA_SYSTEM_TABLE *SYSTEM_TABLE = (SUPERNOVA_SYSTEM_TABLE *) SYSTEM_ADDRESS;

void kprint_cpu()
{
	char brand[51];
	cpuid_brand(brand);
	brand[48] = 0;
	simple_output("CPU #");
	simple_output_number(cpu_local_apic_id());
	outchar(' ');
	simple_output(brand);
	outchar('\n');
}
QWORD core_mapping(QWORD addr)
{
	return addr | SYSTEM_ADDRESS;
}
unsigned long long _DllMainCRTStartup()
{
	__setrbp(__getrsp() & ~((1ULL << 12) - 1));
	setup_paging();
	setup_segment();
	return ((QWORD (*)()) core_mapping((QWORD) coreCRTStartup))();
}
QWORD coreCRTStartup()
{
	setup_interrupe();
	setup_page_fault();
	setup_basic_console();
	setup_memory();
	setup_console();

	simple_output("Supernova OS\n");

	setup_processor();
	setup_apic();

	kprint_cpu();

	setup_acpi();
	setup_timer();
	apic_setup_multiprocessor();
	setup_pcie();


	simple_output("OK\n");

	*((DWORD *) 0x100000000ULL) = 1;

	while (1) __halt();
	return 0;
}