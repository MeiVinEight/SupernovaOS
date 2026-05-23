#pragma once

#include  <types.h>
#include <fs/gpt/gpt.h>

#define NTFS_FILE_SIGNATURE 0x454C4946 // "FILE"

typedef struct _NTFS_BOOT
{
	BYTE  JUMP[3];
	char  NTFS[8];
	BYTE  SSCT[2];
	BYTE  SCLU;
	BYTE  RSV0[7];
	BYTE  MEDA;
	WORD  RSV1;
	DWORD CCHS;
	DWORD HIDN;
	DWORD RSV2;
	DWORD RSV3;
	QWORD SIZE; // Partition Sector Count
	QWORD MFT0;
	QWORD MFT1;
	BYTE  FRSZ;
	BYTE  RSV4[3];
	BYTE  IXSZ;
	BYTE  RSV5[3];
	QWORD SERN; // Serial Number
	DWORD CHCK;
	BYTE  CODE[426];
	WORD  ENDS;
} NTFS_BOOT;
typedef struct _NTFS_MFT_FILE_HEADER
{
	DWORD signature; // "FILE"
	WORD fixupOffset;
	WORD fixupCount;
	QWORD logSeqNum;
	WORD seqNum;
	WORD hardLinkCount;
	WORD attrOffset;
	WORD flags;
	DWORD usedSize;
	DWORD allocatedSize;
	QWORD baseFileRecord;
	WORD nextAttrId;
	WORD reserved;
	DWORD mftRecordNum;
	WORD updateSeqNum;
	WORD updateSeq[2];
} NTFS_MFT_FILE_HEADER;
typedef struct _NTFS_MFT_FILE_RECORD
{
	NTFS_MFT_FILE_HEADER HEAD;
	BYTE RV00[454];
	WORD UPA0;
	BYTE RV01[510];
	WORD UPA1;
} NTFS_MFT_FILE_RECORD;
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
typedef struct _NTFS_FILE
{
	QWORD FMFT;
	NTFS_MFT_ATTR_FILE_NAME *NAME;
	NTFS_MFT_ATTR_HEADER *DATA;
} NTFS_FILE;
typedef struct _NTFS_PARTITION
{
	GUID_PARTITION GUID;
	NTFS_FILE MFTF;
	NTFS_BOOT *BOOT;
	void *MFT0;
} NTFS_PARTITION;

void ntfs_create(GUID_PARTITION *part);
NTFS_MFT_FILE_RECORD *ntfs_mft_record(NTFS_PARTITION *part, DWORD mftNum, void *buf);