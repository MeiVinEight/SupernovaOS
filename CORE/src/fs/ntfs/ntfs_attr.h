#pragma once

#include <types.h>

typedef struct _NTFS_MFT_ATTR_HEADER
{
	DWORD type;
	DWORD length;
	BYTE isNonResident;
	BYTE nameLen;
	WORD nameOffset;
	WORD flags;
	WORD attrId;
	union
	{
		struct
		{
			DWORD valueLen;
			WORD valueOffset;
			BYTE indexedFlag;
			BYTE padding;
		} resident;
		struct
		{
			QWORD startVCN;
			QWORD lastVCN;
			WORD dataRunOffset;
			WORD compressionUnitSize;
			BYTE padding[4];
			QWORD allocatedSize;
			QWORD dataSize;
			QWORD initializedSize;
		} nonResident;
	};
} NTFS_MFT_ATTR_HEADER;
typedef struct _NTFS_MFT_ATTR_FILE_NAME
{
	QWORD parentDirMFTNum;
	QWORD creationTime;
	QWORD fileModTime;
	QWORD mftModTime;
	QWORD accessTime;
	QWORD allocatedSize;
	QWORD realSize;
	DWORD flags;
	DWORD reparse;
	BYTE nameLen;
	BYTE nameType;
	WORD name[];
} NTFS_MFT_ATTR_FILE_NAME;

void *ntfs_attr_body(NTFS_MFT_ATTR_HEADER *attrHeader, int nonResident);
QWORD ntfs_file_name_parent_mft(NTFS_MFT_ATTR_FILE_NAME *fileName);