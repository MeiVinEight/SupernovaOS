#include <fs/part.h>
#include <intrinsic.h>
#include <mm/vmm.h>
#include <fs/ntfs/ntfs.h>
#include <core.h>
#include <interrupt/syscall.h>

COREAPI GUID_PARTITION *PARTITION[26]; // At most 26 partitions, A-Z
COREAPI volatile DWORD COUNTER = 0;

void setup_part_table(STANDARD_STORAGE_DEVICE *disk)
{
	QWORD phyAddr = alloc_physical_memory(1, 0);
	void *buf = (void *) core_mapping(phyAddr);
	DWORD cc = disk->READ(disk, buf, 1, 1);
	if (cc)
		return;
	GUID_PARTITION_TABLE_HEADER header = *(GUID_PARTITION_TABLE_HEADER *) buf;
	QWORD plba = header.PLBA;
	for (DWORD pidx = 0; pidx < header.PCNT; pidx++)
	{
		if (!(pidx & 3))
		{
			cc = disk->READ(disk, buf, plba, 1);
			if (cc)
				return;
			plba++;
		}
		GUID_PARTITION_TABLE_ENTRY *entry = buf;
		entry += pidx & 3;

		DWORD partType = gpt_part_type(entry->UID0);
		if (partType == GPT_PART_TYPE_NULL)
			continue;

		GUID_PARTITION *part = heap_alloc(sizeof(GUID_PARTITION));
		__memset(part, 0, sizeof(GUID_PARTITION));
		__memcpy(part, entry, GUID_PART_HEAD_SIZE);
		part->PIDX = pidx;
		part->DISK = disk;
		disk->PART[pidx] = part;

		if (partType == GPT_PART_TYPE_NTFS)
			ntfs_create(part);

		if (entry->UID1[0] == SYSTEM_TABLE->GUID1[0] && entry->UID1[1] == SYSTEM_TABLE->GUID1[1])
		{
			part->DRIV = 'C';
			PARTITION[2] = part;
		}
	}
	free_physical_memory(phyAddr, 1);
	COUNTER++;
}
BYTE partition_enumerate(QWORD disk, GUID_PARTITION *buff, BYTE max)
{
	if (__getcs() & 3)
	{
		SYSCALL_PARTITION_ENUMERATE arg;
		arg.TYPE = SYSCALL_TYPE_PARTITION_ENUM;
		arg.HNDL = disk;
		arg.ADDR = buff;
		arg.CONT = max;
		return __syscall(&arg);
	}

	STANDARD_STORAGE_DEVICE *ssd = (STANDARD_STORAGE_DEVICE *) disk;
	BYTE cnt = 0;
	for (int i = 0; i < 128; i++)
	{
		if (!ssd->PART[i])
			continue;
		if (buff)
		{
			if (cnt >= max)
				break;
			buff[cnt] = *(GUID_PARTITION *) (ssd->PART[i]);
		}
		cnt++;
	}
	return cnt;
}
DWORD partition_volume(BYTE c, GUID_PARTITION *part)
{
	if (__getcs() & 3)
	{
		SYSCALL_PARTITION_VOLUME arg;
		arg.TYPE = SYSCALL_TYPE_PARTITION_VOLUME;
		arg.CHAR = c;
		arg.PART = part;
		return __syscall(&arg);
	}

	if (!part)
		return 1;
	if (c >= 26)
		return 2;
	__memset(part, 0, sizeof(GUID_PARTITION));
	if (!PARTITION[c])
		return 3;
	__memcpy(part, PARTITION[c], sizeof(GUID_PARTITION));
	return 0;
}