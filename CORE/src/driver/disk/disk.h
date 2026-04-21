#pragma once

#include <types.h>

#define STORAGE_OPERATIO_READ  0
#define STORAGE_OPERATIO_WRITE 1

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
QWORD storage_enumerate(QWORD curr, QWORD *handles, DWORD *count);
QWORD storage_operation(QWORD handle, void *buf, QWORD lba, DWORD sector, DWORD opera);