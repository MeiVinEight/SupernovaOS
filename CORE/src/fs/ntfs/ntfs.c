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
	__memcpy(part->BOOT, buf, 512);
}
void ntfs_resolve_mft(NTFS_PARTITION *part)
{
	part->MFT0 = heap_alloc(1024);
	// __memcpy(part->MFT0, buffer, 1024);
	if (!ntfs_mft_record(part, 0, part->MFT0))
	{
		heap_free(part->MFT0);
		part->MFT0 = 0;
		return;
	}

	ntfs_resolve_record(part->MFT0, &part->FMFT);
}
QWORD ntfs_convert_lcn(NTFS_MFT_ATTR_HEADER *dataAttr, QWORD vcn)
{
	QWORD dataRunOffset = (QWORD) dataAttr + dataAttr->nonResident.dataRunOffset;
	QWORD dataRunEnd = (QWORD) dataAttr + dataAttr->length;
	QWORD prevLcn = 0;
	while (dataRunOffset < dataRunEnd)
	{
		BYTE header = *((BYTE *) dataRunOffset++);
		if (!header)
			break;
		BYTE cluCntSize = header & 0xF;
		BYTE cluOffSize = header >> 4;
		QWORD cluCnt = 0;
		for (BYTE i = 0; i < cluCntSize; i++)
			cluCnt |= ((QWORD) *((BYTE *) dataRunOffset++)) << (i << 3);
		QWORD cluOff = 0;
		for (BYTE i = 0; i < cluOffSize; i++)
			cluOff |= ((QWORD) *((BYTE *) dataRunOffset++)) << (i << 3);
		if (cluOff & (1ULL << (cluOffSize * 8 - 1)))
			cluOff |= ~((1ULL << (cluOffSize << 3)) - 1);
		prevLcn += cluOff;
		if (vcn < cluCnt)
			return prevLcn + vcn;
		vcn -= cluCnt;
	}
	return 0;
}
NTFS_MFT_FILE_RECORD *ntfs_mft_record(NTFS_PARTITION *part, DWORD mftNum, void *buf)
{
	NTFS_BOOT *ntfs = part->BOOT;
	DWORD mftPreClus = ntfs->SCLU >> 1;
	if (!mftPreClus)
		return 0;
	QWORD mftSector = ntfs->HIDN + (ntfs->SCLU * ntfs->MFT0);
	if (mftNum)
	{
		DWORD vcn = mftNum / mftPreClus;
		DWORD lcn = ntfs_convert_lcn(part->FMFT.DATA, vcn);
		DWORD mftInClus = mftNum % mftPreClus;
		mftSector = ntfs->HIDN + (lcn * ntfs->SCLU) + (mftInClus << 1);
	}
	QWORD ret = storage_operation((QWORD) part->GUID.DISK, buf, mftSector, 2, STORAGE_OPERATIO_READ);
	if (ret)
		return 0;
	NTFS_MFT_FILE_RECORD *record = buf;
	record->UPA0 = record->HEAD.updateSeq[0];
	record->UPA1 = record->HEAD.updateSeq[1];
	return record;
}
void ntfs_resolve_record(void *record, NTFS_FILE *file)
{
	NTFS_MFT_FILE_HEADER *mftrecord = record;
	BYTE *attr = (BYTE *) mftrecord;
	attr += mftrecord->attrOffset;
	DWORD attrLen = 0;
	for (;; attr += attrLen)
	{
		NTFS_MFT_ATTR_HEADER *attrHeader = (NTFS_MFT_ATTR_HEADER *) attr;
		if (attrHeader->type == NTFS_ATTR_TYPE_NONE)
			break;
		attrLen = attrHeader->length;
		if (attrHeader->type == NTFS_ATTR_TYPE_FILE_NAME)
		{
			file->NAME = heap_alloc(attrHeader->length);
			__memcpy(file->NAME, attrHeader, attrHeader->length);
		}
		if (attrHeader->type == NTFS_ATTR_TYPE_DATA)
		{
			file->DATA = heap_alloc(attrHeader->length);
			__memcpy(file->DATA, attrHeader, attrHeader->length);
		}
	}
}