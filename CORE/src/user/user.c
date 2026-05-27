#include <user/user.h>
#include <intrinsic.h>
#include <stdio.h>
#include <core.h>
#include <console.h>
#include <mm/vmm.h>
#include <proc/proc.h>
#include <driver/disk/disk.h>
#include <fs/gpt/gpt.h>
#include <fs/part.h>
#include <fs/ntfs/ntfs.h>

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

	{
		DWORD dcnt = -1UL;
		storage_enumerate(0, 0, &dcnt);
		printf("%lu disk enumerate\n", dcnt);
		QWORD *disks = heap_alloc(dcnt * sizeof(QWORD));
		storage_enumerate(0, disks, &dcnt);
		void *addr = 0;
		virtual_alloc(CURRENT_PROCESS_HANDLE, (QWORD *) &addr, 1, VMM_TYPE_COMMITXF);
		GUID_PARTITION *part = heap_alloc(sizeof(GUID_PARTITION) * 16);
		char *buf = heap_alloc(64);
		char *model = buf;
		char *serial = buf + 41;
		for (DWORD didx = 0; didx < dcnt; didx++)
		{
			QWORD disk = disks[didx];
			storage_identify(disk, model, serial);
			printf("%p: %s", (void *) disk, model);
			if (serial[0])
				printf(" - %s", serial);
			printf("\n");
			BYTE pcnt = partition_enumerate(disk, part, 16);
			for (DWORD pidx = 0; pidx < pcnt; pidx++)
				printf("HD(%lu,%lu): %016llX-%016llX\n", didx, pidx, part[pidx].LBA0, part[pidx].LBA1);
		}
		heap_free(part);
	}


	void *buffer = 0;
	virtual_alloc(CURRENT_PROCESS_HANDLE, (QWORD *) &buffer, 1, VMM_TYPE_COMMITXF);


	NTFS_PARTITION *part = heap_alloc(sizeof(NTFS_PARTITION));
	__memset(part, 0, sizeof(NTFS_PARTITION));
	partition_volume(2, &part->GUID);
	ntfs_setup(part);

	/*
	BYTE *dataRun = ((BYTE *) part->FMFT.DATA) + part->FMFT.DATA->nonResident.dataRunOffset;
	BYTE *dataEnd = ((BYTE *) part->FMFT.DATA) + part->FMFT.DATA->length;
	printf("$MFT DATA:");
	while (dataRun < dataEnd)
	{
		printf(" %02X", *dataRun++);
	}
	printf("\n");
	*/

	NTFS_MFT_ATTR_HEADER *attr = part->FMFT.DATA;
	printf("$MFT size: %llu > %llu\n", attr->nonResident.allocatedSize, attr->nonResident.dataSize);
	QWORD mftcnt = attr->nonResident.dataSize >> 10;
	NTFS_FILE file;
	__memset(&file, 0, sizeof(NTFS_FILE));
	char *fileNameA = heap_alloc(266);
	for (QWORD mftidx = 1; mftidx < mftcnt; mftidx++)
	{
		ntfs_mft_record(part, mftidx, buffer);
		NTFS_MFT_FILE_HEADER *fileHeader = buffer;
		if (fileHeader->signature != NTFS_FILE_SIGNATURE)
			goto CONTINUE;
		ntfs_resolve_record(&file, buffer);
		if (!file.NAME)
			goto CONTINUE;

		NTFS_MFT_ATTR_FILE_NAME *fileName = ntfs_attr_body(file.NAME, 0);
		if (ntfs_file_name_parent_mft(fileName) != 5)
			goto CONTINUE;
		for (BYTE i = 0; i < fileName->nameLen; i++)
			fileNameA[i] = (char) fileName->name[i];
		fileNameA[fileName->nameLen] = 0;
		printf("%04llx: %s\n", mftidx, fileNameA);

		CONTINUE:;
		ntfs_file_destructor(&file);
	}


	printf("OK\n");
	while (SYSTEM_TABLE->RUNN) _mm_pause();
}
