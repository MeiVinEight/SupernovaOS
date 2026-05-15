#include <fs/ntfs/ntfs.h>
#include <mm/vmm.h>
#include <intrinsic.h>
#include <driver/disk/disk.h>

void ntfs_create(GUID_PARTITION *part)
{
	if (gpt_part_type(part->TYPE) != GPT_PART_TYPE_NTFS)
		return;
	STANDARD_STORAGE_DEVICE *disk = part->DISK;
	BYTE *buf = storage_dma_buffer(disk);
	__memset(buf, 0, 512);
	storage_operation((QWORD) disk, buf, part->LBA0, 1, STORAGE_OPERATIO_READ);
	__memcpy(part->PART, buf, 512);
}