#include <driver/usb/xhci/xhci_device.h>
#include <driver/usb/xhci/xhci_context.h>
#include <driver/usb/xhci/xhci.h>
#include <driver/usb/usb_desc.h>
#include <memory/virtmem.h>
#include <core.h>
#include <intrinsic.h>
#include <console.h>
#include <driver/usb/usb_req.h>
#include <timer/timer.h>

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
	simple_output("Device Input Context: ");
	simple_output_address(inputCtxPhy, 16);
	outchar('\n');
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
	xhc_transfer_ring_create(&device->transfer, device->input, controller->capability->CSZE, slotId);

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
	volatile XHCI_INPUT_CONTROL_CONTEXT32 *control = xhci_context_get(device->input, -1, device->transfer.CX64);
	simple_output("Input Control Context: ");
	simple_output_address((QWORD) control, 16);
	outchar('\n');
	// Enable A0 (Slot Context) and A1 (Endpoint Control Context: EP Context 0)
	control->ADDX = 3;
	control->DROP = 0;

	volatile XHCI_SLOT_CONTEXT32 *slot = xhci_context_get(device->input, 0, device->transfer.CX64);
	simple_output("Input Slot Context: ");
	simple_output_address((QWORD) slot, 16);
	outchar('\n');
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
				slot->MTTT = ((XHCI_SLOT_CONTEXT32 *) xhci_context_get(hub->input, 0, hub->transfer.CX64))->MTTT;
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

	simple_output("xHCI: slot ");
	simple_output_number(device->slot);
	simple_output(" input ctx: route=");
	simple_output_number(slot->RSTR);
	simple_output(" speed=");
	simple_output_number(slot->SPED);
	simple_output(" root port=");
	simple_output_number(slot->RHPN);
	simple_output(" mps=");
	simple_output_number(maxPs);
	outchar('\n');

	volatile XHCI_ENDPOINT_CONTEXT32 *endpoint0 = xhci_context_get(device->input, 1, device->transfer.CX64);
	simple_output("Input Control Endpoint Context: ");
	simple_output_address((QWORD) endpoint0, 16);
	outchar('\n');
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
	endpoint0->TRDP = physical_address((QWORD) device->transfer.RING);
	endpoint0->TRDP |= device->transfer.CYCL;
	simple_output("Transfer DP: ");
	simple_output_address(endpoint0->TRDP, 16);
	outchar('\n');
}
DWORD xhci_address_device(const XHCI_USB_DEVICE *device, XHCI_TRB_COMMAND_COMPLETION *completion, const DWORD bsr)
{
	volatile XHCI_TRB_ADDRESS_DEVICE trb;
	__memset(&trb, 0, sizeof(XHCI_TRB_ADDRESS_DEVICE));
	trb.CTXT = physical_address((QWORD) device->input);
	simple_output("Address Device With Input Context: ");
	simple_output_address(trb.CTXT, 16);
	outchar('\n');

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
DWORD xhci_send_control_transfer(volatile XHCI_USB_DEVICE *device, USB_DEVICE_SETUP_DATA *requ, void *buf, QWORD len)
{
	PCI_EXPRESS_XHCI_CONTROLLER *controller = device->controller;
	volatile XHCI_TRANSFER_RING *transfer = &device->transfer;

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
	simple_output("Is IN: ");
	simple_output_number(isIn);
	outchar('\n');

	// For OUT data stage, copy caller data into DMA buffer enqueue
	simple_output("Persistent: ");
	simple_output_address(device->persistent, 16);
	outchar('\n');
	void *dmaBuffer = (void *) core_mapping(device->persistent);
	simple_output("DMA: ");
	simple_output_address((QWORD) dmaBuffer, 16);
	outchar('\n');
	if (len && !isIn && buf)
		__memcpy(dmaBuffer, buf, len);
	else
		__memset(dmaBuffer, 0, 0x1000);

	// Setup Stage TRB
	volatile XHCI_TRB_SETUP_STAGE setup;
	__memset(&setup, 0, sizeof(XHCI_TRB_SETUP_STAGE));
	setup.TYPE = XHCI_TRB_TYPE_SETUP_STAGE;
	__memcpy(&setup, &requ, sizeof(USB_DEVICE_SETUP_DATA));
	setup.TTRL = 8;
	setup.INTT = 0;
	setup.IDAT = 1;
	setup.IONC = 0;
	// TRT: 0=No Data, 2=OUT Data, 3=IN Data
	setup.TRTY = (len) ? (isIn ? 3 : 2) : 0;

	simple_output("Setup Stage: ");
	void *stage = xhc_queue_transfer(transfer, (void *) &setup);
	simple_output_address((QWORD) stage, 16);
	outchar('\n');
	if (len)
	{
		volatile XHCI_TRB_DATA_STAGE data;
		__memset(&data, 0, sizeof(XHCI_TRB_DATA_STAGE));
		data.TYPE = XHCI_TRB_TYPE_DATA_STAGE;
		data.DATA = device->persistent;
		data.TTRL = len;
		data.DIRE = isIn;

		simple_output("Data Stage: ");
		stage = xhc_queue_transfer(transfer, (void *) &data);
		simple_output_address((QWORD) stage, 16);
		outchar('\n');
	}

	volatile XHCI_TRB_STATUS_STAGE status;
	__memset(&status, 0, sizeof(XHCI_TRB_STATUS_STAGE));
	status.TYPE = XHCI_TRB_TYPE_STATUS_STAGE;
	status.IONC = 1;
	status.DIRE = (len) ? (isIn ^ 1) : 1;
	simple_output("Status Stage: ");
	stage = xhc_queue_transfer(transfer, (void *) &status);
	simple_output_address((QWORD) stage, 16);
	outchar('\n');

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

	simple_output("Doorbell\n");
	xhc_control_doorbell(controller->doorbell, device->slot);
	return 0;
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
void xhci_usb_enumerate_device(XHCI_USB_DEVICE *device)
{
	volatile PCI_EXPRESS_XHCI_CONTROLLER *controller = device->controller;
	DWORD isRoot = !device->route;

	// Configure the input context for Address Device command
	DWORD psiv = device->speed;
	DWORD initMaxPs = xhci_usb_initial_max_packet_size(psiv);
	xhci_usb_configure_control_endpoint(device, initMaxPs);

	/*
	volatile QWORD *ctx = (QWORD *) controller->context[device->slot];
	simple_output("Device Context: ");
	simple_output_address((QWORD) ctx, 16);
	outchar('\n');
	ctx = (QWORD *) core_mapping((QWORD) ctx);
	for (int i = 0; i < 8; i++)
	{
		simple_output_address(ctx[i], 16);
		outchar('\n');
	}

	simple_output("Device Input Context: ");
	simple_output_address((QWORD) device->input, 16);
	outchar('\n');
	for (int i = 0; i < 12; i++)
	{
		simple_output_address(((volatile QWORD *) device->input)[i], 16);
		outchar('\n');
	}
	*/

	// First address device with BSR=1, essentially blocking the SET_ADDRESS request,
	// but still enables the control endpoint which we can use to get the device descriptor.
	// Some legacy devices require their descriptor to be read before sending them a SET_ADDRESS command.
	XHCI_TRB_COMMAND_COMPLETION completion;
	__memset(&completion, 0, sizeof(XHCI_TRB_COMMAND_COMPLETION));
	DWORD code = xhci_address_device(device, &completion, 0);
	if (code != XHCI_CODE_SUCCESS)
	{
		simple_output("xHCI: Address Device Command failed: ");
		simple_output_number(code);
		outchar('\n');
		return;
	}

	/*
	simple_output("Device Context: ");
	simple_output_address((QWORD) ctx, 16);
	outchar('\n');
	for (int i = 0; i < 8; i++)
	{
		simple_output_address(ctx[i], 16);
		outchar('\n');
	}
	*/

	// Read the first 8 bytes of the device descriptor to get bMaxPacketSize0.
	// On VL805/VL817 the hub TT can babble under concurrent periodic traffic,
	// so retry with CLEAR_TT_BUFFER between attempts.
	STANDARD_USB_DEVICE desc;
	__memset(&desc, 0, sizeof(STANDARD_USB_DEVICE));
	simple_output("xHCI: GET DESCRITPRO 3 times\n");
	DWORD rc = xhci_get_device_descriptor(device, &desc, 8);
	if (rc)
	{
		simple_output("xHCI: Failed to get device descriptor: ");
		simple_output_number(rc);
		simple_output(" for slot ");
		simple_output_number(device->slot);
		outchar('\n');
		return;
	}

	delay(1000);

	simple_output("USB DEVICE: ");
	simple_output_address(*((volatile QWORD *) &desc), 16);
	outchar('\n');

	/*
	simple_output("Device Context: ");
	simple_output_address((QWORD) ctx, 16);
	outchar('\n');
	for (int i = 0; i < 8; i++)
	{
		simple_output_address(ctx[i], 16);
		outchar('\n');
	}
	*/
}