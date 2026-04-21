#include <driver/disk/disk.h>

STANDARD_STORAGE_DEVICE *volatile STORAGE_DEVICE;

void storage_insert(STANDARD_STORAGE_DEVICE *device)
{
	device->NEXT = STORAGE_DEVICE;
	STORAGE_DEVICE = device;
}