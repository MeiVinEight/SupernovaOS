#pragma once

#include <types.h>
#include <fs/gpt/gpt.h>
#include <driver/disk/disk.h>

extern GUID_PARTITION *PARTITION[];

void setup_part_table(STANDARD_STORAGE_DEVICE *disk);
BYTE partition_enumerate(QWORD disk, GUID_PARTITION *buff, BYTE max);