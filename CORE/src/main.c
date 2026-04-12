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
#include <stdio.h>
#include <driver/usb/usb_keyboard.h>

QWORD __stdcall coreCRTStartup();

COREAPI volatile SUPERNOVA_SYSTEM_TABLE *SYSTEM_TABLE = (SUPERNOVA_SYSTEM_TABLE *) SYSTEM_ADDRESS;

void kprint_cpu()
{
	char brand[51];
	cpuid_brand(brand);
	brand[48] = 0;
	printf("CPU #%u %s\n", cpu_local_apic_id(), brand);
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
	setup_processor();
	return ((QWORD (*)()) core_mapping((QWORD) coreCRTStartup))();
}
QWORD coreCRTStartup()
{
	setup_interrupt();
	setup_page_fault();
	setup_basic_console();
	setup_memory();
	setup_console();

	simple_output("Supernova OS\n");
	kprint_cpu();
	SYSTEM_TABLE->CRUN = 1;
	printf("Video: %lux%lu Memory: %lx\n", SYSTEM_TABLE->PPL, (DWORD) SYSTEM_TABLE->VRES, SYSTEM_TABLE->VRES * SYSTEM_TABLE->PPL * 4);

	setup_apic();
	setup_acpi();
	setup_timer();
	apic_setup_multiprocessor();
	setup_system_call();


	setup_pcie();


	simple_output("OK\n");

	while (SYSTEM_TABLE->CRUN)
	{
		xhci_keyboard_process();
	}
	return 0;
}