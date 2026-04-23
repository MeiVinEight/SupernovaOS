#pragma once

#include <types.h>

typedef struct _GUID_PARTITION_TABLE_HEADER
{
	// Identifies EFI-compatible partition table header. This value must contain the ASCII string "EFI PART", encoded as the 64-bit constant 0x54 52415020494645.
	QWORD SIGN;
	// The revision number for this header. This revision value is not related to the UEFI Specification version. This header is version 1.0, so the correct value is 0x00010000.
	DWORD REVI;
	// Size in bytes of the GPT Header. The HeaderSize must be greater than or equal to 92 and must be less than or equal to the logical block size.
	DWORD HSZE;
	// CRC32 checksum for the GPT Header structure. This value is computed by setting this field to 0, and computing the 32-bit CRC for HeaderSize bytes.
	DWORD HCRC;
	// Must be zero
	DWORD RSV0;
	// The LBA that contains this data structure.
	QWORD LBA1;
	// LBA address of the alternate GPT Header.
	QWORD ALBA;
	// The first usable logical block that may be used by a partition described by a GUID Partition Entry.
	QWORD FLBA;
	// The last usable logical block that may be used by a partition described by a GUID Partition Entry.
	QWORD LLBA;
	// GUID that can be used to uniquely identify the disk.
	QWORD GUID[2];
	// The starting LBA of the GUID Partition Entry array.
	QWORD PLBA;
	// The number of Partition Entries in the GUID Partition Entry array.
	DWORD PCNT;
	/**
	 * The size, in bytes, of each the GUID Partition Entry structures in the GUID Partition Entry array.
	 *
	 * This field shall be set to a value of 128 x 2 n where n is an integer greater than or equal to zero (e.g., 128, 256, 512, etc.).
	 *
	 * NOTE: Previous versions of this specification allowed any multiple of 8..
	 */
	DWORD PSZE;
	// The CRC32 of the GUID Partition Entry array. Starts at Par titionEntryLBA and is computed over a byte length of NumberOfP artitionEntries * SizeOfP artitionEntry.
	DWORD PCRC;
	// The rest of the block is reserved by UEFI and must be zero.
	BYTE  RSV1[0x1A4];
} GUID_PARTITION_TABLE_HEADER;
typedef struct _GUID_PARTITION_TABLE_ENTRY
{
	// Unique ID that defines the purpose and type of this Partition. A value of zero defines that this partition entry is not being used.
	QWORD UID0[2];
	/**
	 * GUID that is unique for every partition entry. Every partition ever created will have a unique GUID.
	 *
	 * This GUID must be assigned when the GPT Partition Entry is created.

	 The GPT Partition Entry is created whenever the NumberOfPa rtitionEntries in the GPT Header is increased to include a larger range of addresses.
	 */
	QWORD UID1[2];
	// Starting LBA of the partition defined by this entry.
	QWORD LBA0;
	// Ending LBA of the partition defined by this entry.
	QWORD LBA1;
	/**
	 * Attribute bits, all bits reserved by UEFI
	 * [Defined GPT Partition Entry — Partition Type GUIDs](https://uefi.org/specs/UEFI/2.10_A/05_GUID_Partition_Table_Format.html#defined-gpt-partition-entry-partition-type-guids).
	 */
	QWORD ATTR;
	// Null-terminated string containing a human-readable name of the partition.
	WORD  NAME[36];
} GUID_PARTITION_TABLE_ENTRY;