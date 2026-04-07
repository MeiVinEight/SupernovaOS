#include <driver/usb/usb_keyboard.h>
#include <driver/usb/usb_req.h>
#include <driver/usb/usb_hid.h>
#include <driver/xhci/xhc_ring.h>
#include <stdio.h>

XHCI_USB_DEVICE *USB_KEYBOARD;

DWORD xhci_usb_keyboard_setup(XHCI_USB_DEVICE *device, STANDARD_USB_INTERFACE *iface)
{
	printf("USB Keyboard @ %u\n", device->slot);

	// Search interrupt in endpoint
	STANDARD_USB_ENDPOINT *endpoint = usb_search_endpoint(device->configuration, USB_XFER_TYPE_INT, USB_DIR_IN);
	if (!endpoint)
	{
		printf("USB: Endpoint Interrupt In NOT FOUND for device %u\n", device->slot);
		return -2;
	}

	printf("USB Endpoint: addr=%02x, attr=%02x, max packet size=%04x, interval=%u\n", endpoint->ADDR, endpoint->ATTR, endpoint->MPSZ, endpoint->ITVL);

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
	if ((cc = xhci_send_control_transfer(device, &requ, 0, 0)) != XHCI_CODE_SUCCESS)
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
	if ((cc = xhci_send_control_transfer(device, &requ, 0, 0)) != XHCI_CODE_SUCCESS)
	{
		printf("xHCI: USB Set Idle failed: %lu\n", cc);
		return 0x80 + cc;
	}

	if (xhci_usb_configure_xfer_endpoint(device, endpoint))
		return 1;
	USB_KEYBOARD = device;
	return 0;
}
void xhci_keyboard_process()
{
	XHCI_USB_DEVICE *device = USB_KEYBOARD;
	if (!device)
		return;

	XHCI_TRANSFER_RING *transfer = 0;
	for (DWORD i = 3; i < 32; i += 2)
	{
		if (device->transfer[i])
		{
			transfer = device->transfer[i];
			break;
		}
	}
	if (!transfer)
		return;
	XHCI_TRB_GENERIC *blk = xhc_event_ring_pop(transfer);
	if (blk != 0)
	{
		printf("Keyboard Event @ %p\n", blk);
	}
}