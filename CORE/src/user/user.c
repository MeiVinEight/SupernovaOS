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
	for (DWORD didx = 0; didx < dcnt; didx++)
	{
		printf("disk %p\n", (void *) disks[didx]);
		DWORD cc = storage_operation(disks[didx], addr, 1, 1, STORAGE_OPERATIO_READ);
		if (cc)
		{
			printf("Disk read failed: %ld\n", cc);
			continue;
		}
		GUID_PARTITION_TABLE_HEADER header = *(GUID_PARTITION_TABLE_HEADER *) addr;
		/*
		if (header.GUID[0] != SYSTEM_TABLE->GUID0[0] || header.GUID[1] != SYSTEM_TABLE->GUID0[1])
			continue;
		*/

		QWORD plba = header.PLBA;
		for (DWORD pidx = 0; pidx < header.PCNT; pidx++)
		{
			if (!(pidx & 3))
			{
				cc = storage_operation(disks[didx], addr, plba, 1, STORAGE_OPERATIO_READ);
				if (cc)
				{
					printf("Disk read failed: %ld\n", cc);
					continue;
				}
				plba++;
			}
			GUID_PARTITION_TABLE_ENTRY *entry = addr;
			entry += pidx & 3;
			if (!(entry->UID0[0] | entry->UID0[1]))
				 break;
			char partName[38];
			__memset(partName, 0, 38);
			printf("HD(%lu,%lu): %016llX+%016llX ", didx, pidx, entry->LBA0, entry->LBA1 - entry->LBA0 + 1);
			int nameLen = 0;
			for (; nameLen < 36 && entry->NAME[nameLen]; nameLen++)
				partName[nameLen] = (char) entry->NAME[nameLen];
			partName[nameLen] = '\n';
			partName[nameLen + 1] = 0;
			printf(partName);
		}
	}
	printf("OK\n");
	while (SYSTEM_TABLE->RUNN) _mm_pause();
}
