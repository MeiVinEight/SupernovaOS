#include <driver/usb/xhci/xhci_context.h>

void *xhci_context_get(void *context, DWORD idx, DWORD cx64)
{
	BYTE *addr = context;
	return (addr + ((0x20 << cx64) * (idx + 1)));
}