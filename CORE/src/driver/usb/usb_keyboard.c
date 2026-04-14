#include <driver/usb/usb_keyboard.h>
#include <driver/usb/usb_req.h>
#include <driver/usb/usb_hid.h>
#include <driver/xhci/xhc_ring.h>
#include <stdio.h>
#include <intrinsic.h>
#include <acpi/fadt.h>
#include <driver/xhci/xhci_context.h>

XHCI_USB_DEVICE *USB_KEYBOARD;
HID_STANDARD_KEYEVENT KEYEVENT;
volatile BYTE KEY_COUNT = 0;

void xhci_usb_keyboard_event(XHCI_TRANSFER_RING *ring)
{
	if (!ring->COMP.CCOD)
		return;
	if (ring->COMP.CCOD != XHCI_CODE_SUCCESS)
	{
		printf("USB Keyboard Error: %u\n", ring->COMP.CCOD);
		return;
	}

	XHCI_USB_DEVICE *device = ring->DEVC;
	for (int i = 0; i < 6; i++)
	{
		if ((KEYEVENT.KEY[i] == KEYEVENT_KEY_ESC) && (KEYEVENT.MOD & KEYEVENT_CTRL_LCTRL))
			acpi_shutdown();
	}
	__memset(&KEYEVENT, 0, sizeof(HID_STANDARD_KEYEVENT));
	KEYEVENT.RSV = 1;
	xhci_transfer(device, ring->EPID, 0, 0, &KEYEVENT, sizeof(HID_STANDARD_KEYEVENT));
}
DWORD xhci_usb_keyboard_setup(XHCI_USB_DEVICE *device, STANDARD_USB_INTERFACE *iface)
{
	// Search interrupt in endpoint
	STANDARD_USB_ENDPOINT *endpoint = usb_search_endpoint(device->configuration, USB_XFER_TYPE_INT, USB_DIR_IN);
	if (!endpoint)
	{
		printf("USB: Endpoint Interrupt In NOT FOUND for device %u\n", device->slot);
		return -2;
	}

	// Enable "boot" protocol.
	USB_DEVICE_SETUP_DATA requ;
	requ.RECP = 1; // Interface
	requ.RTYP = 1; // Class
	requ.DIRE = 0; // Host to Device
	requ.REQU = HID_REQ_SET_PROTOCOL;
	requ.VALU = 0;
	requ.INDX = iface->IFCN;
	requ.LENG = 0;
	DWORD cc;
	if ((cc = xhci_control_transfer(device, &requ, 0, 0)) != XHCI_CODE_SUCCESS)
	{
		printf("xHCI: USB Set Protocol failed: %lu\n", cc);
		return 0x80 + cc;
	}

	// Periodically send reports to enable key repeat.
	// Send USB HID SetIdle request
	requ.RECP = USB_RECIP_INTERFACE; // Interface
	requ.RTYP = USB_RTYPE_CLASS;
	requ.DIRE = USB_RDIR_OUT;
	requ.REQU = HID_REQ_SET_IDLE;
	requ.VALU = (500 / 4) << 8;
	requ.INDX = 0;
	requ.LENG = 0;
	if ((cc = xhci_control_transfer(device, &requ, 0, 0)) != XHCI_CODE_SUCCESS)
	{
		printf("xHCI: USB Set Idle failed: %lu\n", cc);
		return 0x80 + cc;
	}

	if (xhci_usb_configure_xfer_endpoint(device, endpoint))
		return 1;
	USB_KEYBOARD = device;
	BYTE epid = xhci_endpoint_id(endpoint);
	device->transfer[epid]->COMP.CCOD = XHCI_CODE_SUCCESS;
	device->transfer[epid]->CALL = xhci_usb_keyboard_event;
	xhci_usb_keyboard_event(device->transfer[epid]);
	return 0;
}