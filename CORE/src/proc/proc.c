#include <proc/proc.h>
#include <mm/vmm.h>
#include <intrinsic.h>
#include <file/pe32x.h>
#include <user/user.h>
#include <core.h>
#include <stdio.h>
#include <mm/pmm.h>

extern BYTE __ImageBase;

PROCESS_CONTROL_BLOCK *volatile CURRENT_PROCESS;

PROCESS_CONTROL_BLOCK *create_process()
{
	PROCESS_CONTROL_BLOCK *pcb = heap_alloc(sizeof(PROCESS_CONTROL_BLOCK));
	__memset(pcb, 0, sizeof(PROCESS_CONTROL_BLOCK));
	vmm_free(&pcb->VMMA, 0x1000, 0x7FFFFFFFFULL);

	IMAGE_DOS_HEADER *dosHeader = (IMAGE_DOS_HEADER *) &__ImageBase;
	IMAGE_NT_HEADERS *ntHeaders = (IMAGE_NT_HEADERS *) ((&__ImageBase) + dosHeader->PEHO);
	DWORD imageSize = ntHeaders->OPTI.SIMG;
	DWORD allocSize = (imageSize + 0xFFF) & ~0xFFF;
	QWORD virtAddr = 0x0000800000000000ULL - allocSize;
	virtual_alloc(physical_address((QWORD) pcb), &virtAddr, allocSize >> 12, VMM_TYPE_COMMIT, VMM_EXECUTE | VMM_WRITE);
	__memcpy((void *) virtAddr, &__ImageBase, imageSize);
	dosHeader = (IMAGE_DOS_HEADER *) (virtAddr);
	ntHeaders = (IMAGE_NT_HEADERS *) (virtAddr + dosHeader->PEHO);
	ntHeaders->OPTI.IMGE = virtAddr;
	ntHeaders->OPTI.ENTY = ((QWORD) user_main) & 0xFFFFFFFF;
	SUPERNOVA_SYSTEM_TABLE *sysTab = (SUPERNOVA_SYSTEM_TABLE *) virtAddr;
	__memcpy(sysTab->APC, sysTab->FONT, 0x1000);
	pcb->CORE = virtAddr;
	return pcb;
}
PROCESS_CONTROL_BLOCK *current_process()
{
	return CURRENT_PROCESS;
}
QWORD process_start(PROCESS_CONTROL_BLOCK *pcb)
{
	CURRENT_PROCESS = pcb;
	IMAGE_DOS_HEADER *dosHeader = (IMAGE_DOS_HEADER *) (pcb->CORE);
	IMAGE_NT_HEADERS *ntHeaders = (IMAGE_NT_HEADERS *) (pcb->CORE + dosHeader->PEHO);
	SUPERNOVA_SYSTEM_TABLE *sysTab = (SUPERNOVA_SYSTEM_TABLE *) pcb->CORE;
	QWORD stack = ((QWORD) sysTab->APC) - 0x40;
	QWORD entry = ntHeaders->OPTI.ENTY + pcb->CORE;
	return __iret(0x13, entry, 0x1B, stack);
}