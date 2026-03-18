#pragma once

#include <intrinsic.h>

typedef struct _NTFS_BPB
{
	BYTE opcode[3];
	BYTE OEM[8];
	BYTE BPS[2];   // byte per sector, always 512
	BYTE cluster;  // sector per cluster
	BYTE reserved1[7];
	BYTE descriptor;
	BYTE reserved2[2];
	WORD track;    // sector per track
	WORD head;
	DWORD hidden;  // hidden sectors
	BYTE reserved3[8];
	QWORD sector;  // sector count
	QWORD MFT;     // $MFT sector
	QWORD MFTMIRR; // $MFTMirr sector
	BYTE FILE;     // FILE size
	BYTE reserved4[3];
	BYTE INDX;     // INDX size
	BYTE reserved5[3];
	QWORD serial;
	DWORD checksum;
} NTFS_BPB;
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

QWORD ntfs_logical_cluster_number(NTFS_MFT_ATTR_HEADER *attr, QWORD vcn);
DWORD ntfs_file_name(NTFS_MFT_ATTR_FILE_NAME *fileName, char *buf);
NTFS_MFT_ATTR_FILE_NAME *ntfs_attr_file_name(void *mft);
NTFS_MFT_ATTR_HEADER *ntfs_attr_data(void *mft);