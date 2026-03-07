#include <intrinsic.h>

COREAPI char paging_address_unaligned[4096 * 7];

unsigned long long _DllMainCRTStartup()
{
	QWORD buf[2] = {0, 0};
	__memset128((void *) 0xF0000000, buf, (1920 * 1080 * 4) / 16);
	while (1);
}