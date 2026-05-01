#include <fs/ntfs/ntfs.h>
#include <mm/vmm.h>
#include <intrinsic.h>
#include <core.h>
#include <driver/disk/disk.h>
#include <stdio.h>

GUID_PARTITION_NTFS *ntfs_create(QWORD handle, GUID_PARTITION_TABLE_ENTRY *gpt)
{
	if (gpt_part_type(gpt->UID0) != GPT_PART_TYPE_NTFS)
		return 0;
	GUID_PARTITION_NTFS *ntfs = heap_alloc(sizeof(GUID_PARTITION_NTFS));
	__memset(ntfs, 0, sizeof(GUID_PARTITION_NTFS));
	__memcpy(ntfs, gpt, GUID_PART_HEAD_SIZE);
	QWORD phyAddr = alloc_physical_memory(1, 0);
	void *buf = (void *) core_mapping(phyAddr);
	__memset(buf, 0, 0x1000);
	storage_operation(handle, buf, ntfs->GUID.LBA0, 1, STORAGE_OPERATIO_READ);
	ntfs->BOOT = *(NTFS_BOOT *) buf;
	printf("%s\n", ntfs->BOOT.NTFS);
	free_physical_memory(phyAddr, 1);
	return ntfs;
}