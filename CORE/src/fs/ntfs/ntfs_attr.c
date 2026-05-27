#include <fs/ntfs/ntfs_attr.h>

void *ntfs_attr_body(NTFS_MFT_ATTR_HEADER *attrHeader, int nonResident)
{
	if (!attrHeader)
		return 0;
	BYTE *data = (BYTE *) attrHeader;
	if (nonResident)
		data += attrHeader->nonResident.dataRunOffset;
	else
		data += attrHeader->resident.valueOffset;
	return data;
}
QWORD ntfs_file_name_parent_mft(NTFS_MFT_ATTR_FILE_NAME *fileName)
{
	return fileName->parentDirMFTNum & ((1ULL << 48) - 1);
}