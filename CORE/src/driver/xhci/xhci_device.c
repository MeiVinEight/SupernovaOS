#include <driver/xhci/xhci_device.h>
#include <driver/xhci/xhci_context.h>
#include <driver/xhci/xhci.h>
#include <driver/usb/usb_desc.h>
#include <memory/virtmem.h>
#include <core.h>
#include <intrinsic.h>
#include <console.h>
#include <driver/usb/usb_req.h>
#include <timer/timer.h>
#include <stdio.h>
#include <driver/usb/usb_keyboard.h>

DWORD xhci_setup_usb_device(XHCI_USB_DEVICE *device, DWORD portId, DWORD slotId)
{
	device->parent = 0;
	PCI_EXPRESS_XHCI_CONTROLLER *controller = device->controller;

	QWORD pc = 1;
	QWORD outCtxPhy = alloc_physical_memory(&pc, 0, 0);
	if (!outCtxPhy)
		return 1;
	__memset((void *) core_mapping(outCtxPhy), 0, pc << 12);
	// Setup Output Context
	controller->context[slotId] = outCtxPhy;

	QWORD inputCtxPhy = alloc_physical_memory(&pc, 0, 0);
	if (!inputCtxPhy)
	{
		free_physical_memory(outCtxPhy, 1);
		controller->context[slotId] = 0;
		return 1;
	}
	// Input Context
	device->input = (void *) core_mapping(inputCtxPhy);
	__memset(device->input, 0, pc << 12);

	// Allocate a persistent DMA page for control transfer payloads.
	pc = 1;
	QWORD persPhyAddr = alloc_physical_memory(&pc, 0, 0);
	if (!persPhyAddr)
	{
		free_physical_memory(outCtxPhy, 1);
		free_physical_memory(inputCtxPhy, 1);
		controller->context[slotId] = 0;
		return 1;
	}
	device->persistent = persPhyAddr;

	// Trasnfer Ring
	xhc_transfer_ring_create(device->transfer[1], 1);

	device->route = 0;
	device->port = portId;
	device->slot = slotId;
	device->speed = controller->operational->PORT[portId].PSPD;
	return 0;
}
DWORD xhci_usb_initial_max_packet_size(DWORD speed)
{
	switch (speed)
	{
		case XHCI_USB_SPEED_HIGH_SPEED:
			return 64;
		case XHCI_USB_SPEED_SUPER_SPEED:
		case XHCI_USB_SPEED_SUPER_SPEED_PLUS:
			return 512;
		case XHCI_USB_SPEED_FULL_SPEED:
		case XHCI_USB_SPEED_LOW_SPEED:
		default:
			return 8;
	}
}
void xhci_usb_configure_control_endpoint(XHCI_USB_DEVICE *device, DWORD maxPs)
{
	PCI_EXPRESS_XHCI_CONTROLLER *controller = device->controller;
	DWORD ctx64 = controller->capability->CSZE;
	volatile XHCI_INPUT_CONTROL_CONTEXT32 *control = xhci_context_get(device->input, -1, ctx64);
	// Enable A0 (Slot Context) and A1 (Endpoint Control Context: EP Context 0)
	control->ADDX = 3;
	control->DROP = 0;

	volatile XHCI_SLOT_CONTEXT32 *slot = xhci_context_get(device->input, 0, ctx64);
	slot->RSTR = device->route;
	slot->SPED = device->speed;
	slot->CENT = 1;
	slot->INTX = 0;
	if (!device->route)
	{
		// Root Hub device: port id is the root hub port number
		slot->RHPN = device->port + 1;
	}
	else
	{
		simple_output("Not root hub\n");
		// Hub-downstream device: use the root port of the topology chain
		slot->RHPN = device->root + 1;

		// xHCI spec Section 6.2.2: parent hub slot id and parent port number
		// shall reference the nearest HS hub providing the TT, only for LS/FS
		// device. Walk up the hub chain to find it.
		if (device->speed == XHCI_USB_SPEED_LOW_SPEED || device->speed == XHCI_USB_SPEED_FULL_SPEED)
		{
			XHCI_USB_DEVICE *hub = device->parent;
			while (hub && hub->speed != XHCI_USB_SPEED_HIGH_SPEED)
				hub = hub->parent;
			if (hub && hub->speed == XHCI_USB_SPEED_HIGH_SPEED)
			{
				slot->PSID = hub->slot;
				slot->PRPN = hub->port + 1;
				slot->MTTT = ((XHCI_SLOT_CONTEXT32 *) xhci_context_get(hub->input, 0, ctx64))->MTTT;
				simple_output("xHCI: slot ");
				simple_output_number(device->slot);
				simple_output(" TT: hub slog=");
				simple_output_number(hub->slot);
				simple_output(" hub port=");
				simple_output_number(hub->port);
				simple_output(" MTT=");
				simple_output_number(slot->MTTT);
				outchar('\n');
			}
			else
			{
				simple_output("xHCI: slot ");
				simple_output_number(device->slot);
				simple_output(" TT: no HS hub found (parent slot ");
				simple_output_number((device->parent) ? (device->parent->slot) : 0);
				simple_output(" speed ");
				simple_output_number((device->parent) ? (device->parent->speed) : 0);
				outchar('\n');
			}
		}
	}

	printf("xHCI: slot:%u, input ctx route:%u, speed:%u, root port:%u, mps:%lu\n", device->slot, slot->RSTR, slot->SPED, slot->RHPN, maxPs);

	XHCI_TRANSFER_RING *transfer = device->transfer[1];

	volatile XHCI_ENDPOINT_CONTEXT32 *endpoint0 = xhci_context_get(device->input, 1, ctx64);
	endpoint0->STAT = XHCI_ENDPOINT_STATE_DISABLED; // 0
	endpoint0->TYPE = XHCI_ENDPOINT_TYPE_CONTROL; // 4
	// Max Packet Size
	endpoint0->MPSZ = maxPs;
	// Max Brush Size
	endpoint0->MBSZ = 0;
	// Error Count
	//endpoint0->CERR = 3;
	// Interval
	endpoint0->ITVL = 0;
	// Average TRB Length
	endpoint0->AVRT = maxPs;
	// Max ESIT Payload Lo
	endpoint0->MEPL = 0;
	// Max ESIT Payload Hi
	endpoint0->MEPH = 0;
	endpoint0->TRDP = physical_address((QWORD) transfer->RING);
	endpoint0->TRDP |= transfer->CYCL;
}
void xhci_usb_configure_xfer_endpoint(XHCI_USB_DEVICE *device, STANDARD_USB_ENDPOINT *endpoint)
{
}
DWORD xhci_address_device(const XHCI_USB_DEVICE *device, XHCI_TRB_COMMAND_COMPLETION *completion, const DWORD bsr)
{
	volatile XHCI_TRB_ADDRESS_DEVICE trb;
	__memset(&trb, 0, sizeof(XHCI_TRB_ADDRESS_DEVICE));
	trb.CTXT = physical_address((QWORD) device->input);

	/*
		Block Set Address Request (BSR). When this flag is set to '0' the Address Device Command shall
		generate a USB SET_ADDRESS request to the device. When this flag is set to '1' the Address
		Device Command shall not generate a USB SET_ADDRESS request. Refer to section 4.6.5 for
		more information on the use of this flag.
	*/
	trb.BSAR = bsr;

	trb.TYPE = XHCI_TRB_TYPE_ADDRESS_DEVICE;
	trb.SLOT = device->slot;
	return xhci_send_command(device->controller, (void *) &trb, completion);
}
DWORD xhci_transfer(XHCI_USB_DEVICE *device, DWORD endpoint, USB_DEVICE_SETUP_DATA *requ, void *buf)
{
	if (!endpoint)
		return -1;
	if (endpoint > 31)
		return -1;

	PCI_EXPRESS_XHCI_CONTROLLER *controller = device->controller;
	XHCI_TRANSFER_RING *transfer = device->transfer[endpoint];
	DWORD len = requ->LENG;

	// Use the device's persisitent DMA buffer
	if (!device->persistent)
	{
		simple_output("xHCI: Missing control transfer buffer for slot ");
		simple_output_number(device->slot);
		outchar('\n');
		return 1;
	}
	if (len > 0x1000)
	{
		simple_output("xHCI: Control trasnfer too large: ");
		simple_output_number(len);
		outchar('\n');
		return 1;
	}

	DWORD isIn = requ->DIRE;

	// For OUT data stage, copy caller data into DMA buffer enqueue
	void *dmaBuffer = (void *) core_mapping(device->persistent);
	if (len && !isIn && buf)
		__memcpy(dmaBuffer, buf, len);
	else
		__memset(dmaBuffer, 0, 0x1000);

	// Setup Stage TRB
	volatile XHCI_TRB_SETUP_STAGE setup;
	__memset(&setup, 0, sizeof(XHCI_TRB_SETUP_STAGE));
	__memcpy(&setup, requ, sizeof(USB_DEVICE_SETUP_DATA));
	setup.TYPE = XHCI_TRB_TYPE_SETUP_STAGE;
	setup.TTRL = 8;
	setup.INTT = 0;
	setup.IDAT = 1;
	setup.IONC = 0;
	// TRT: 0=No Data, 2=OUT Data, 3=IN Data
	setup.TRTY = (len) ? (isIn ? 3 : 2) : 0;

	xhc_queue_transfer(transfer, (void *) &setup);
	if (len)
	{
		volatile XHCI_TRB_DATA_STAGE data;
		__memset(&data, 0, sizeof(XHCI_TRB_DATA_STAGE));
		data.TYPE = XHCI_TRB_TYPE_DATA_STAGE;
		data.DATA = device->persistent;
		data.TTRL = len;
		data.DIRE = isIn;

		xhc_queue_transfer(transfer, (void *) &data);
	}

	volatile XHCI_TRB_STATUS_STAGE status;
	__memset(&status, 0, sizeof(XHCI_TRB_STATUS_STAGE));
	status.TYPE = XHCI_TRB_TYPE_STATUS_STAGE;
	status.IONC = 1;
	status.DIRE = (len) ? (isIn ^ 1) : 1;
	xhc_queue_transfer(transfer, (void *) &status);

	// VL805 quirk: avoid ringing the doorbell near the SOF boundary for
	// FS non-periodic transfers behind a hub (TT babble avoidance).
	/*
	if (device->route && device->speed == XHCI_USB_SPEED_FULL_SPEED)
	{
		for (DWORD tries = 0; tries < 20; tries++)
		{
			if (controller->runtime->MFRM & 7)
				break;
			__halt();
		}
	}
	*/

	volatile XHCI_TRB_TRANSFER_EVENT *xfer = (XHCI_TRB_TRANSFER_EVENT *) &transfer->COMP;
	xfer->CCOD = 0;
	xhc_ring_doorbell(controller->doorbell, device->slot, endpoint);
	while (!xfer->CCOD) delay(1);
	__memcpy(buf, dmaBuffer, len);
	return xfer->CCOD;
}
DWORD xhci_send_control_transfer(volatile XHCI_USB_DEVICE *device, USB_DEVICE_SETUP_DATA *requ, void *buf, QWORD len)
{
	return xhci_transfer((XHCI_USB_DEVICE *) device, 1, requ, buf);
}
DWORD xhci_get_device_descriptor(XHCI_USB_DEVICE *device, void *out, DWORD len)
{
	USB_DEVICE_SETUP_DATA requ;
	__memset(&requ, 0, sizeof(USB_DEVICE_SETUP_DATA));
	requ.RECP = 0; // Device
	requ.RTYP = 0; // Standard
	requ.DIRE = 1; // Device to Host
	requ.REQU = USB_REQ_GET_DESCRIPTOR;
	requ.VALU = USB_DESC_DEVICE << 8;
	requ.INDX = 0;
	requ.LENG = len;
	return xhci_send_control_transfer(device, &requ, out, len);
}
DWORD xhci_get_config_descriptor(XHCI_USB_DEVICE *device, DWORD indx, void *out, DWORD len)
{
	if (indx > 0xFF)
		return -1;
	USB_DEVICE_SETUP_DATA requ;
	__memset(&requ, 0, sizeof(USB_DEVICE_SETUP_DATA));
	requ.RECP = 0; // Device
	requ.RTYP = 0; // Standard
	requ.DIRE = 1; // Device To Host
	requ.REQU = USB_REQ_GET_DESCRIPTOR;
	requ.VALU = (USB_DESC_CONFIGURATION << 8) | indx;
	requ.INDX = 0;
	requ.LENG = len;
	return xhci_send_control_transfer(device, &requ, out, len);
}
DWORD xhci_get_string_descriptor(XHCI_USB_DEVICE *device, DWORD indx, STANDARD_USB_STRING *out, DWORD len)
{
	if (indx > 0xFF)
		return -1;
	USB_DEVICE_SETUP_DATA requ;
	__memset(&requ, 0, sizeof(USB_DEVICE_SETUP_DATA));
	requ.RECP = 0; // Device
	requ.RTYP = 0; // Standard
	requ.DIRE = 1; // Device To Host
	requ.REQU = USB_REQ_GET_DESCRIPTOR;
	requ.VALU = (USB_DESC_STRING << 8) | indx;
	requ.INDX = 0;
	requ.LENG = len;
	return xhci_send_control_transfer(device, &requ, out, len);
}
void xhci_usb_enumerate_device(XHCI_USB_DEVICE *device)
{
	PCI_EXPRESS_XHCI_CONTROLLER *controller = device->controller;
	DWORD isRoot = !device->route;

	// Configure the input context for Address Device command
	DWORD psiv = device->speed;
	DWORD initMaxPs = xhci_usb_initial_max_packet_size(psiv);
	xhci_usb_configure_control_endpoint(device, initMaxPs);

	XHCI_TRB_COMMAND_COMPLETION completion;
	__memset(&completion, 0, sizeof(XHCI_TRB_COMMAND_COMPLETION));
	DWORD rc = xhci_address_device(device, &completion, 0);
	if (rc != XHCI_CODE_SUCCESS)
	{
		simple_output("xHCI: Address Device Command failed: ");
		simple_output_number(rc);
		outchar('\n');
		return;
	}

	// Read the first 8 bytes of the device descriptor to get bMaxPacketSize0.
	// On VL805/VL817 the hub TT can babble under concurrent periodic traffic,
	// so retry with CLEAR_TT_BUFFER between attempts.
	STANDARD_USB_DEVICE desc;
	__memset(&desc, 0, sizeof(STANDARD_USB_DEVICE));
	rc = xhci_get_device_descriptor(device, &desc, 8);
	if (rc != XHCI_CODE_SUCCESS)
	{
		simple_output("xHCI: Failed to get device descriptor: ");
		simple_output_number(rc);
		simple_output(" for slot ");
		simple_output_number(device->slot);
		outchar('\n');
		return;
	}

	printf("USB Descriptor: len=%u, type=%u, rel=%x, class=%u, subclass=%u, proto=%u, maxps=%u\n", desc.LENG, desc.TYPE, desc.UREL, desc.CCOD, desc.SCOD, desc.POTO, desc.MPS0);
	// If the device reported a different max packet size, update the input context
	DWORD reportMaxPs = desc.MPS0;
	if (reportMaxPs == 9)
		reportMaxPs = 512;
	if (reportMaxPs != initMaxPs)
	{
		xhci_usb_configure_control_endpoint(device, reportMaxPs);

		// Send Evaluate Context to update the xHC's internal state
		printf("Evalute Context for Max Packet Size: %lu\n", reportMaxPs);
		XHCI_TRB_EVALUATE_CONTEXT evaluate;
		__memset(&evaluate, 0, sizeof(XHCI_TRB_EVALUATE_CONTEXT));
		evaluate.CTXT = physical_address((QWORD) device->input);
		evaluate.TYPE = XHCI_TRB_TYPE_EVALUATE_CONTEXT;
		evaluate.SLOT = device->slot;
		if ((rc = xhci_send_command(controller, &evaluate, 0)) != XHCI_CODE_SUCCESS)
		{
			printf("Evaluate Context failed: %lu\n", rc);
			return;
		}
	}

	/*
	USB 2.0 Spec 9.2.6.3
	After successful completion of the Status stage, the device is allowed a SetAddress() recovery interval of
	2 ms. At the end of this interval, the device must be able to accept Setup packets addressed to the new
	address. Also, at the end of the recovery interval, the device must not respond to tokens sent to the old
	address (unless, of course, the old and new address is the same).
	 */
	delay(2);

	printf("Device State: %u\n", ((XHCI_SLOT_CONTEXT32 *) controller->context[device->slot])->STAT);

	__memset(&desc, 0, sizeof(STANDARD_USB_DEVICE));
	rc = xhci_get_device_descriptor(device, &desc, sizeof(STANDARD_USB_DEVICE));
	if (rc != XHCI_CODE_SUCCESS)
	{
		printf("xHCI: Failed to get full device descriptor: %lu for slot %u\n", rc, device->slot);
		return;
	}
	printf("USB Vendor:%04X Product:%04X, %u config(s)\n", desc.VNID, desc.PRID, desc.CNFN);

	STANDARD_USB_CONFIGURATION iconf;
	__memset(&iconf, 0, sizeof(STANDARD_USB_CONFIGURATION));
	if ((rc = xhci_get_config_descriptor(device, 0, &iconf, 9)) != XHCI_CODE_SUCCESS)
	{
		printf("xHCI: Failed to get standard configuration descriptor: %lu\n", rc);
		return;
	}
	STANDARD_USB_CONFIGURATION *conf = heap_alloc(iconf.TLEN);
	__memset(conf, 0, iconf.TLEN);
	if ((rc = xhci_get_config_descriptor(device, 0, conf, iconf.TLEN)) != XHCI_CODE_SUCCESS)
	{
		printf("xHCI: Failed to get total configuration descriptor: %lu\n", rc);
		return;
	}
	printf("USB: slot %u config: %u interface(s)\n", device->slot, conf->IFAC);
	device->configuration = conf;

	// Sync the input context with the xHC's current output context
	// so slot and EP0 state are up-to-date before we add new endpoints
	DWORD ctx64 = controller->capability->CSZE;
	__memcpy(((XHCI_SLOT_CONTEXT32 *) device->input) + (1 << ctx64), (void *) controller->context[device->slot], 32 * (sizeof(XHCI_SLOT_CONTEXT32) << ctx64));

	// Reset input control context flags (clear stale bits from ADDRESS_DEVICE)
	XHCI_INPUT_CONTROL_CONTEXT32 *ictx = device->input;
	ictx->DROP = 0;
	ictx->ADDX = 1;

	DWORD offset = 0;
	DWORD dataLen = (conf->TLEN > 9) ? (conf->TLEN - 9) : 0;
	STANDARD_USB_INTERFACE *iface = 0;
	while (offset + 2 <= dataLen)
	{
		iface = (STANDARD_USB_INTERFACE *) (conf->DATA + offset);
		if (iface->LENG < 2)
			goto NEXT;
		if (iface->TYPE == USB_DESC_INTERFACE && iface->LENG >= 9)
			break;

		NEXT:
		offset += iface->LENG;
	}

	USB_DEVICE_SETUP_DATA requ;
	requ.RECP = 0; // Device
	requ.RTYP = 0; // Standard
	requ.DIRE = 0; // Host to Device
	requ.REQU = USB_REQ_SET_CONFIGURATION;
	requ.VALU = conf->CNFV;
	requ.INDX = 0;
	requ.LENG = 0;
	if ((rc = xhci_send_control_transfer(device, &requ, 0, 0)) != XHCI_CODE_SUCCESS)
	{
		printf("xHCI: USB Set Configuration failed: %lu\n", rc);
		return;
	}

	if (!iface)
	{
		printf("xHCI: USB Interface NOT FOUND for slot %u\n", device->slot);
		return;
	}

	printf("USB Interface: num=%u, setting=%u, endpoint=%u, class=%02x, subclass=%02x, proto=%u, iface=%u\n",
		iface->IFCN, iface->SETT, iface->ENDP, iface->CCOD, iface->SCOD, iface->POTO, iface->IFAC);
	if (iface->CCOD == USB_CLASS_HID)
		xhci_usb_hid_setup(device, iface);
}
DWORD xhci_usb_hid_setup(XHCI_USB_DEVICE *device, STANDARD_USB_INTERFACE *iface)
{
	// Doesn't support boot protocol.
	if (iface->SCOD != USB_INTERFACE_SUBCLASS_BOOT)
		return -1;
	if (iface->POTO == USB_INTERFACE_PROTOCOL_KEYBOARD)
		return xhci_usb_keyboard_setup(device, iface);
	return -1;
}