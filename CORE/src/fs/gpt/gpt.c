#include <fs/gpt/gpt.h>
#include <intrinsic.h>

COREAPI QWORD PART_GUID_NTFS [] = {0x4433B9E5EBD0A0A2, 0xC79926B7B668C087};
COREAPI QWORD PART_GUID_FAT32[] = {0x11D2F81FC12A7328, 0x3BC93EC9A0004BBA};

DWORD gpt_part_type(QWORD *guid)
{
	if (!(guid[0] | guid[1]))
		return GPT_PART_TYPE_NULL;
	if (guid[0] == PART_GUID_NTFS[0] && guid[1] == PART_GUID_NTFS[1])
		return GPT_PART_TYPE_NTFS;
	if (guid[0] == PART_GUID_FAT32[0] && guid[1] == PART_GUID_FAT32[1])
		return GPT_PART_TYPE_FAT32;
	return GPT_PART_TYPE_UNKNOWN;
}