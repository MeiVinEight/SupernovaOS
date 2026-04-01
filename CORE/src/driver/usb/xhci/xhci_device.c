#include <driver/usb/xhci/xhci_device.h>
#include <driver/usb/xhci/xhci.h>
#include <console.h>

DWORD setup_usb_device(XHCI_USB_DEVICE *device)
{
	PCI_EXPRESS_XHCI_DEVICE *controller = (PCI_EXPRESS_XHCI_DEVICE *) device->controller;
	return 1;
}