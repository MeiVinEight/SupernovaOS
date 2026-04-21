#include <driver/usb/usb_msc.h>
#include <driver/usb/usb_bulk.h>
#include <driver/xhci/xhci_context.h>
#include <stdio.h>

void xhci_usb_msc_setup(XHCI_USB_DEVICE *device)
{
	STANDARD_USB_INTERFACE *iface = device->interface;
	if (!iface)
		return;
	if (iface->SCOD == USB_MSC_SUBCLASS_SCSI)
	{
		if (iface->POTO == USB_MSC_PROTOCOL_BOT)
			xhci_usb_msc_bot(device);
	}
}