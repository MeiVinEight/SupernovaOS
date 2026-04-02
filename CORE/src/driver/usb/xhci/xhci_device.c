#include <driver/usb/xhci/xhci_device.h>
#include <driver/usb/xhci/xhci_context.h>
#include <driver/usb/xhci/xhci.h>
#include <memory/virtmem.h>
#include <core.h>
#include <intrinsic.h>
#include <console.h>

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
		case XHCI_USB_SPEED_FULL_SPEED:
		case XHCI_USB_SPEED_HIGH_SPEED:
			return 64;
		case XHCI_USB_SPEED_SUPER_SPEED:
		case XHCI_USB_SPEED_SUPER_SPEED_PLUS:
			return 512;
		case XHCI_USB_SPEED_LOW_SPEED:
		default:
			return 8;
	}
}
void xhci_usb_configure_control_endpoint(XHCI_USB_DEVICE *device, DWORD maxPs)
{
	XHCI_INPUT_CONTROL_CONTEXT32 *control = xhci_context_get(device->input, -1, device->transfer.CX64);
	// Enable A0 (Slot Context) and A1 (Endpoint Control Context: EP Context 0)
	control->ADDX = 3;

	XHCI_SLOT_CONTEXT32 *slot = xhci_context_get(device->input, 0, device->transfer.CX64);
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

	XHCI_ENDPOINT_CONTEXT32 *endpoint0 = xhci_context_get(device->input, 1, device->transfer.CX64);
	endpoint0->STAT = XHCI_ENDPOINT_STATE_DISABLED;
	endpoint0->TYPE = XHCI_ENDPOINT_TYPE_CONTROL;
	endpoint0->MPSZ = maxPs;
	endpoint0->MBSZ = 0;
	endpoint0->CERR = 3;
	endpoint0->ITVL = 0;
	endpoint0->AVRT = 8;
	endpoint0->MEPL = 0;
	endpoint0->TRDP = physical_address((QWORD) device->transfer.RING);
	endpoint0->TRDP |= device->transfer.CYCL;
}
void xhci_usb_enumerate_device(XHCI_USB_DEVICE *device)
{
	DWORD isRoot = !device->route;

	// Configure the input context for Address Device command
	DWORD psiv = device->speed;
	DWORD initMaxPs = xhci_usb_initial_max_packet_size(psiv);
	xhci_usb_configure_control_endpoint(device, initMaxPs);

	// First address device with BSR=1, essentially blocking the SET_ADDRESS request,
	// but still enables the control endpoint which we can use to get the device descriptor.
	// Some legacy devices require their descriptor to be read before sending them a SET_ADDRESS command.
}