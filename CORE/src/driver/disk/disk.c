#include <driver/disk/disk.h>
#include <interrupt/syscall.h>
#include <intrinsic.h>
#include <fs/part.h>

STANDARD_STORAGE_DEVICE *volatile STORAGE_DEVICE;

void storage_insert(STANDARD_STORAGE_DEVICE *device)
{
	device->NEXT = STORAGE_DEVICE;
	STORAGE_DEVICE = device;
	setup_part_table(device);
}
QWORD storage_enumerate(QWORD curr, QWORD *handles, DWORD *count)
{
	if (!count)
		return -1;

	if (__getcs() & 3)
	{
		SYSCALL_STORAGE_ENUMERATE arg;
		arg.TYPE = SYSCALL_TYPE_STORAGE_ENUM;
		arg.CURR = curr;
		arg.HNDL = handles;
		arg.CONT = count;
		return __syscall(&arg);
	}

	STANDARD_STORAGE_DEVICE *dev = (STANDARD_STORAGE_DEVICE *) curr;
	if (!dev)
		dev = STORAGE_DEVICE;
	DWORD cnt = 0;
	while (dev)
	{
		if (cnt >= *count)
			break;
		if (handles)
			handles[cnt] = (QWORD) dev;
		cnt++;
		dev = dev->NEXT;
	}
	*count = cnt;
	return 0;
}
QWORD storage_operation(QWORD handle, void *buf, QWORD lba, DWORD sector, DWORD opera)
{
	if (__getcs() & 3)
	{
		SYSCALL_STORAGE_OPERATION arg;
		arg.TYPE = SYSCALL_TYPE_STORAGE_OPER;
		arg.HNDL = handle;
		arg.ADDR = buf;
		arg.LBAA = lba;
		arg.CONT = sector;
		arg.OPER = opera;
		return __syscall(&arg);
	}
	STANDARD_STORAGE_DEVICE *device = (STANDARD_STORAGE_DEVICE *) handle;
	if (opera == STORAGE_OPERATIO_READ)
		return device->READ(device, buf, lba, sector);
	if (opera == STORAGE_OPERATIO_WRITE)
		return device->WRIT(device, buf, lba, sector);
	return -1;
}