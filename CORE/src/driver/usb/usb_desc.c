#include <driver/usb/usb_desc.h>

STANDARD_USB_ENDPOINT *usb_search_endpoint(STANDARD_USB_CONFIGURATION *conf, int type, int dir)
{
	STANDARD_USB_ENDPOINT *endpoint = 0;
	DWORD offset = 0;
	DWORD dataLen = (conf->TLEN > 9) ? (conf->TLEN - 9) : 0;
	while (offset + 2 <= dataLen)
	{
		STANDARD_USB_ENDPOINT *endp = (STANDARD_USB_ENDPOINT *) (conf->DATA + offset);
		if (endp->LENG < 2)
			goto NEXT;
		if (endp->TYPE != USB_DESC_ENDPOINT)
			goto NEXT;
		if (endp->LENG < 7)
			goto NEXT;
		if ((endp->ADDR & USB_ENDPOINT_MASK_DIR) != dir)
			goto NEXT;
		if ((endp->ATTR & USB_ENDPOINT_XFER_TYPE) != type)
			goto NEXT;
		endpoint = endp;
		break;

		NEXT:
		offset += endp->LENG;
	}
	return endpoint;
}
