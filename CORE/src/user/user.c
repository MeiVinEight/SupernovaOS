#include <user/user.h>
#include <intrinsic.h>
#include <stdio.h>
#include <core.h>
#include <console.h>
#include <mm/vmm.h>
#include <proc/proc.h>
#include <driver/disk/disk.h>
#include <fs/gpt/gpt.h>
#include <timer/timer.h>
#include <fs/part.h>

extern BYTE __ImageBase;

void user_main()
{
	SYSTEM_TABLE = (SUPERNOVA_SYSTEM_TABLE *) &__ImageBase;
	SIMPLE_TEXT.COLOR = 0x0A;
	printf("SupernovaOS @ %p\n", SYSTEM_TABLE);
	SIMPLE_TEXT.COLOR = 0x0F;

	// Create Process Heap
	{
		HEAPK = 0;
		QWORD heapSpace = (1ULL << 40 /*1TB*/) - (1ULL << 30 /*1GB*/);
		virtual_alloc(CURRENT_PROCESS_HANDLE, (QWORD *) &HEAPK, heapSpace >> 12, VMM_TYPE_COMMITXF);
		HEAPK[0] = (heapSpace - 8) | HEAP_FLAG_LAST;
	}

	DWORD dcnt = -1UL;
	storage_enumerate(0, 0, &dcnt);
	printf("%lu disk enumerate\n", dcnt);
	QWORD *disks = heap_alloc(dcnt * sizeof(QWORD));
	storage_enumerate(0, disks, &dcnt);
	void *addr = 0;
	virtual_alloc(CURRENT_PROCESS_HANDLE, (QWORD *) &addr, 1, VMM_TYPE_COMMITXF);
	GUID_PARTITION *part = heap_alloc(sizeof(GUID_PARTITION) * 16);
	for (DWORD didx = 0; didx < dcnt; didx++)
	{
		printf("disk %p\n", (void *) disks[didx]);
		BYTE pcnt = partition_enumerate(disks[didx], part, 16);
		for (DWORD pidx = 0; pidx < pcnt; pidx++)
		{
			printf("HD(%lu,%lu): %016llX-%016llX\n", didx, pidx, part[pidx].LBA0, part[pidx].LBA1);
		}
	}
	printf("OK\n");
	while (SYSTEM_TABLE->RUNN) _mm_pause();
}
