#pragma once

#include <types.h>
#include <fs/ntfs/ntfs_attr.h>

#define NTFS_INDEX_FLG_LAST 2

typedef struct _NTFS_INDEX_HEADER
{
	DWORD OFST; // Offset to first index record
	DWORD SZE0; // Index reocrd total size
	DWORD SZE1; // Index reocrd allocated size
	DWORD FLAG;
} NTFS_INDEX_HEADER;
typedef struct _NTFS_INDEX_RECORD
{
	QWORD MFT0; // MFT Reference for this file
	WORD  SZE0; // Size of index entry
	WORD  SZE1; // Size of filename attr
	DWORD FLAG;
	NTFS_MFT_ATTR_FILE_NAME NAME;
} NTFS_INDEX_RECORD;

NTFS_INDEX_RECORD *ntfs_next_index_record(NTFS_INDEX_RECORD *record);