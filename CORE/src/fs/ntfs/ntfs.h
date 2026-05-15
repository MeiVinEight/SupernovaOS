#pragma once

#include  <types.h>
#include <fs/gpt/gpt.h>

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
	QWORD SIZE; // Paritiion Sector Count
	QWORD MFT0;
	QWORD MFT1;
	BYTE  FRSZ;
	BYTE  RSV4[3];
	BYTE  IXSZ;
	BYTE  RSV5[3];
	QWORD SERN; // Serial Number
	DWORD CHCK;
} NTFS_BOOT;
typedef struct _GUID_PARTITION_NTFS
{
	GUID_PARTITION GUID;
	NTFS_BOOT      BOOT;
} GUID_PARTITION_NTFS;

void ntfs_create(GUID_PARTITION *part);