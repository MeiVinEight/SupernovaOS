#include <ntfs.h>

QWORD ntfs_logical_cluster_number(NTFS_MFT_ATTR_HEADER *attr, QWORD vcn)
{
	QWORD dataRunOffset = (QWORD) attr + attr->nonResident.dataRunOffset;
	QWORD dataRunEnd = (QWORD) attr + attr->length;
	QWORD prevLcn = 0;
	while (dataRunOffset < dataRunEnd)
	{
		BYTE header = *((BYTE *) dataRunOffset++);
		if (!header)
			break;
		BYTE clusterCountSize = header & 0xF;
		BYTE clusterOffsetSize = header >> 4;
		QWORD clusterCount = 0;
		for (BYTE i = 0; i < clusterCountSize; i++)
			clusterCount |= ((QWORD) *((BYTE *) dataRunOffset++)) << (i * 8);
		QWORD clusterOffset = 0;
		for (BYTE i = 0; i < clusterOffsetSize; i++)
			clusterOffset |= ((QWORD) *((BYTE *) dataRunOffset++)) << (i * 8);
		if (clusterOffset & ((QWORD) 1 << (clusterOffsetSize * 8 - 1)))
			clusterOffset |= ~((1ULL << (clusterOffsetSize * 8)) - 1);
		prevLcn += clusterOffset;
		if (vcn < clusterCount)
			return prevLcn + vcn;
		vcn -= clusterCount;
	}
	return 0;
}
DWORD ntfs_file_name(NTFS_MFT_ATTR_FILE_NAME *fileName, char *buf)
{
	buf[fileName->nameLen] = 0;
	for (BYTE i = 0; i < fileName->nameLen; i++)
		buf[i] = fileName->name[i] & 0xFF;
	return fileName->nameLen;
}
NTFS_MFT_ATTR_FILE_NAME *ntfs_attr_file_name(void *mft)
{
	NTFS_MFT_FILE_HEADER *file = (NTFS_MFT_FILE_HEADER *) mft;
	if (file->signature != 0x454C4946) // "FILE"
		return 0;

	BYTE *attribute = (BYTE *) mft;
	attribute += file->attrOffset;

	while (1)
	{
		NTFS_MFT_ATTR_HEADER *attrHeader = (NTFS_MFT_ATTR_HEADER *) attribute;
		if (attrHeader->type == 0xFFFFFFFF)
			break;
		if (attrHeader->type != 0x0030)
			goto NEXT_ATTR;

		NTFS_MFT_ATTR_FILE_NAME *fileName = (NTFS_MFT_ATTR_FILE_NAME *) (attribute + attrHeader->resident.valueOffset);
		if (!(fileName->nameType & 1))
			goto NEXT_ATTR;

		return fileName;

		NEXT_ATTR:
		attribute += *((DWORD *) (attribute + 4));
	}
	return 0;
}
NTFS_MFT_ATTR_HEADER *ntfs_attr_data(void *mft)
{
	NTFS_MFT_FILE_HEADER *file = (NTFS_MFT_FILE_HEADER *) mft;
	if (file->signature != 0x454C4946) // "FILE"
		return 0;

	BYTE *attribute = (BYTE *) mft;
	attribute += file->attrOffset;

	while (1)
	{
		NTFS_MFT_ATTR_HEADER *attrHeader = (NTFS_MFT_ATTR_HEADER *) attribute;
		if (attrHeader->type == 0xFFFFFFFF)
			break;
		if (attrHeader->type != 0x0080)
			goto NEXT_ATTR;

		return attrHeader;

		NEXT_ATTR:
		attribute += attrHeader->length;
	}
	return 0;
}