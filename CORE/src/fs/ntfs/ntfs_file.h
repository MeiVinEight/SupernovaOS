#pragma once

#include <types.h>
#include <fs/ntfs/ntfs_attr.h>

typedef struct _NTFS_FILE
{
	QWORD FMFT;
	NTFS_MFT_ATTR_HEADER *NAME;
	NTFS_MFT_ATTR_HEADER *DATA;
} NTFS_FILE;

void ntfs_resolve_record(NTFS_FILE *file, void *record);
void ntfs_file_destructor(NTFS_FILE *file);