#include <driver/usb/usb_keyboard.h>
#include <driver/usb/usb_req.h>
#include <driver/usb/usb_hid.h>
#include <stdio.h>

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

	
	return 0;
}