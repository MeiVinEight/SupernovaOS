#include <driver/xhci/xhci_context.h>

DWORD xhci_endpoint_id(STANDARD_USB_ENDPOINT *endpoint)
{
	return ((endpoint->ADDR << 1) | (endpoint->ADDR >> 7)) & 0x1F;
}