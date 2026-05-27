#include <fs/ntfs/ntfs_file.h>
#include <fs/ntfs/ntfs.h>
#include <mm/vmm.h>
#include <intrinsic.h>
#include <stdio.h>

void ntfs_resolve_record(NTFS_FILE *file, void *record)
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
			NTFS_MFT_ATTR_FILE_NAME *fileName = ntfs_attr_body(attrHeader, attrHeader->isNonResident);
			if (!fileName)
				continue;
			if (!(fileName->nameType & 1))
				continue;
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
void ntfs_file_destructor(NTFS_FILE *file)
{
	heap_free(file->NAME);
	heap_free(file->DATA);
	__memset(file, 0, sizeof(NTFS_FILE));
}