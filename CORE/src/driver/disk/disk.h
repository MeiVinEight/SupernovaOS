#pragma once

#include <types.h>

typedef struct _STANDARD_STORAGE_DEVICE STANDARD_STORAGE_DEVICE;
struct _STANDARD_STORAGE_DEVICE
{
	STANDARD_STORAGE_DEVICE *NEXT;
	QWORD                  (*READ)(STANDARD_STORAGE_DEVICE *device, void *buf, QWORD lba, DWORD sector);
	QWORD                  (*WRIT)(STANDARD_STORAGE_DEVICE *device, void *buf, QWORD lba, DWORD sector);
	QWORD                    CAPA;
};

extern STANDARD_STORAGE_DEVICE *volatile STORAGE_DEVICE;

void storage_insert(STANDARD_STORAGE_DEVICE *device);