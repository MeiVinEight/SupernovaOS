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
#include <file/pe32x.h>
#include <user/user.h>

extern BYTE __ImageBase;
QWORD __stdcall coreCRTStartup();

COREAPI volatile SUPERNOVA_SYSTEM_TABLE *SYSTEM_TABLE = (SUPERNOVA_SYSTEM_TABLE *) SYSTEM_ADDRESS;

void kprint_cpu()
{
	char brand[51];
	cpuid_brand(brand);
	brand[48] = 0;
	printf("CPU #%u %s\n", cpu_local_apic_id(), brand);
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
	SYSTEM_TABLE->PAGING[0][0] = 0;
	setup_interrupt();
	setup_page_fault();
	setup_basic_console();
	setup_memory();
	setup_console();

	simple_output("Supernova OS\n");
	kprint_cpu();
	SYSTEM_TABLE->RUNN = 1;
	printf("Video: %lux%lu Memory: %lx\n", SYSTEM_TABLE->PPL, (DWORD) SYSTEM_TABLE->VRES, SYSTEM_TABLE->VRES * SYSTEM_TABLE->PPL * 4);

	setup_apic();
	setup_acpi();
	setup_timer();
	apic_setup_multiprocessor();
	setup_system_call();
	setup_pcie();


	simple_output("OK\n");

	IMAGE_DOS_HEADER *dosHeader = (IMAGE_DOS_HEADER *) &__ImageBase;
	DWORD userMainOffset = ((QWORD) user_main) & 0xFFFFFFFF;
	IMAGE_NT_HEADERS *ntHeaders = (IMAGE_NT_HEADERS *) ((&__ImageBase) + dosHeader->PEHO);
	DWORD imageSize = ntHeaders->OPTI.SIMG;
	QWORD virtAddr = 0x00007FFF00000000ULL;
	QWORD size = 0;
	while (size < imageSize)
	{
		QWORD pc = 1;
		QWORD phyAddr = alloc_physical_memory(&pc, 0, 0);
		virtual_mapping(phyAddr, virtAddr + size, 1, PAGE_4K, PA_WRITE | PA_USER);
		size += 0x1000;
	}
	__memcpy((void *) virtAddr, &__ImageBase, imageSize);
	// Modify first page DOS HEADER not writable
	paging_attribute(virtAddr, PA_USER);
	dosHeader = (IMAGE_DOS_HEADER *) (virtAddr);
	ntHeaders = (IMAGE_NT_HEADERS *) (virtAddr + dosHeader->PEHO);
	ntHeaders->OPTI.IMGE = virtAddr;
	ntHeaders->OPTI.ENTY = userMainOffset;
	QWORD userMain = userMainOffset + virtAddr;
	SUPERNOVA_SYSTEM_TABLE *sysTab = (SUPERNOVA_SYSTEM_TABLE *) virtAddr;
	__memcpy(sysTab->APC, sysTab->FONT, 0x1000);
	QWORD userStack = (QWORD) sysTab->APC;
	userStack -= 0x20;
	return __iret(0x13, userMain, 0x1B, userStack);
}