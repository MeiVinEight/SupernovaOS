#include <fs/ntfs/ntfs_index.h>

NTFS_INDEX_RECORD *ntfs_next_index_record(NTFS_INDEX_RECORD *record)
{
	if (!record)
		return 0;
	if (record->FLAG == NTFS_INDEX_FLG_LAST)
		return 0;
	return (NTFS_INDEX_RECORD *) ((BYTE *) record + record->SZE0);
}
