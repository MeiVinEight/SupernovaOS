#include <driver/usb/usb_msc.h>
#include <driver/usb/usb_bulk.h>
#include <driver/xhci/xhci_context.h>

DWORD xhci_usb_msc_setup(XHCI_USB_DEVICE *device)
{
	STANDARD_USB_INTERFACE *iface = device->interface;
	if (!iface)
		return 1;
	if (iface->SCOD == USB_MSC_SUBCLASS_SCSI)
	{
		if (iface->POTO == USB_MSC_PROTOCOL_BOT)
			return xhci_usb_msc_bot(device);
	}
	return 1;
}