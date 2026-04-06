#include <driver/usb/xhci/xhci.h>
#include <console.h>
#include <core.h>
#include <intrinsic.h>
#include <memory/virtmem.h>
#include <driver/usb/xhci/xhc_regs.h>
#include <driver/pci/pci.h>
#include <interrupt/apic.h>
#include <driver/usb/xhci/xhci_port.h>
#include <driver/usb/xhci/xhci_device.h>
#include <stdio.h>
#include <timer/timer.h>

COREAPI PCI_EXPRESS_XHCI_CONTROLLER DEVICE;
COREAPI volatile XHCI_USB_DEVICE USB_DEVICE;

void setup_usb_xhci_pcie(PCI_EXPRESS_DEVICE *dev)
{
	PCI_DEVICE_VENDOR vendor;
	vendor.VENDOR = dev->configuration->vendor;
	vendor.DEVICE = dev->configuration->device;
	printf("PCI Express @ %016llX: %06X - ", (QWORD) dev->configuration, dev->configuration->class);
	const char *vendorName = pci_vendor_name(vendor.VENDOR);
	const char *deviceName = pci_device_name(vendor);
	if (vendorName && deviceName)
		printf("%s %s", vendorName, deviceName);
	else
		simple_output_address(vendor.ID, 8);
	outchar('\n');

	pcie_setup_interrupt(dev, xhci_interrupt, interrupt_alloc_intx());

	//device.pcie = *dev;
	__memcpy(&DEVICE.pcie, dev, sizeof(PCI_EXPRESS_DEVICE));

	QWORD xhciBase = pcie_cfg_get_base_address(dev, 0);
	DEVICE.address = xhciBase;
	DEVICE.capability = (XHCI_CAPABILITY_SPACE *) core_mapping(xhciBase);
	DEVICE.operational = (XHCI_OPERATIONAL_SPACE *) core_mapping(xhciBase + DEVICE.capability->SIZE);
	DEVICE.runtime = (XHCI_RUNTIME_SPACE *) core_mapping(xhciBase + DEVICE.capability->RTME);
	DEVICE.doorbell = (XHCI_DOORBELL *) core_mapping(xhciBase + DEVICE.capability->BELL);

	DWORD reset = xhci_reset_controller(&DEVICE);
	if (!reset)
	{
		simple_output("Reset failed\n");
		return;
	}

	xhci_configure_controller(&DEVICE);

	if (!xhci_start_controller(&DEVICE))
	{
		simple_output("XHCI start failed\n");
		return;
	}

	__halt();
	__halt();
	DWORD maxprt = DEVICE.capability->PORT;
	for (DWORD i = 0; i < maxprt; i++)
	{
		if (!DEVICE.operational->PORT[i].CCSS)
			continue;

		printf("Port %lu\n", i);
		xhci_setup_device(&DEVICE, i);
	}
}
DWORD xhci_reset_controller(PCI_EXPRESS_XHCI_CONTROLLER *device)
{
	// Clear Run/Stop bit
	device->operational->RNST = 0;

	// Wait for halt bit set
	while (!device->operational->HALT) __halt();

	// Set HC Reset bit
	device->operational->HRST = 1;

	// Wait for RST and CNT bit clear
	while (device->operational->HRST || device->operational->HCNR) __halt();

	// brief delay
	__halt();

	// Check registers all bits clear
	if (device->operational->UCMD)
		return 0;

	if (device->operational->DNCR)
		return 0;

	if (device->operational->CRCR)
		return 0;

	if (device->operational->CBAA)
		return 0;

	if (device->operational->UCFG)
		return 0;

	return 1;
}
DWORD xhci_start_controller(PCI_EXPRESS_XHCI_CONTROLLER *device)
{
	// Ensure USBCMD bit for RUN/STOP is properly set
	device->operational->INTE = 1;
	device->operational->RNST = 1;

	// Ensure the controller transistions out of the halted state
	while (device->operational->HALT) __halt();

	// Verify CNR (Controller Not Ready) bit is clear
	if (device->operational->HCNR)
		return 0;

	// Controller started successfully
	return 1;
}
void xhci_configure_controller(PCI_EXPRESS_XHCI_CONTROLLER *device)
{
	/* ==== Operational ==== */
	// Enable device notification
	device->operational->DNCR = 0xFFFF;

	// Configure the usbconfig filed
	BYTE maxSlot = device->capability->SLOT;
	device->operational->MDSE = maxSlot;

	//Setup the device context base address array with scratchpad buffers
	QWORD dcbaaSize = sizeof(void *) * (device->capability->SLOT + 1);
	QWORD pageCount = 1;
	QWORD dcbaAddr = alloc_physical_memory(&pageCount, 0, 0);
	device->context = (QWORD *) core_mapping(dcbaAddr);
	__memset(device->context, 0, 0x1000);

	/*

	xHCI Specification 6.1

	If the Max Scratchpad Buffers field of the HCSPARAMS2 register is > ‘0’, then the
	first entry (entry_0) in the DCBAA shall contain a pointer to the Scratchpad
	Buffer Array. If the Max Scratchpad Buffers field of the HCSPARAMS2 register is
	= ‘0’, then the first entry (entry_0) in the DCBAA is reserved and shall be cleared
	to ‘0’ by software.

	*/
	DWORD maxScratchpadBuf = (device->capability->MSBH << 5) | device->capability->MSBL;
	if (maxScratchpadBuf)
	{
		// TODO Check PAGESIZE in operational registers
		QWORD pcnt = maxScratchpadBuf << 3;
		pcnt += 0xFFF;
		pcnt >>= 12;
		QWORD scrArrAddr = alloc_physical_memory(&pcnt, 0, 0);
		QWORD *scrpadArr = (QWORD *) core_mapping(scrArrAddr);
		__memset(scrpadArr, 0, pcnt << 12);
		// Create scratchpad pages
		pcnt = 1;
		for (DWORD i = 0; i < maxScratchpadBuf; i++)
		{
			scrpadArr[i] = alloc_physical_memory(&pcnt, 0, 0);
			__memset((void *) core_mapping(scrpadArr[i]), 0, pcnt << 12);
		}

		// Set the first slot in the DCBAA to point to the scratchpad array
		device->context[0] = scrArrAddr;
	}

	// Set DCBAA Address into operational DCBAA registr
	device->operational->CBAA = dcbaAddr;

	// Setup command ring, CRCR
	xhc_transfer_ring_create(&device->command, 1);
	device->operational->CRCR = physical_address((QWORD) device->command.RING) | 1;
	/* ==== Operational ==== */


	/* ==== Runtime ==== */
	// Setup the event ring and write to interrupter
	// registers to se ERSTSZ, ERDP, and ERSTBA.
	xhc_transfer_ring_create(&device->event, 0);

	// Get the primary interrupter registers
	volatile XHCI_INTERRUPTER *intr = device->runtime->INTR;

	// Enable interrupts
	intr->IENA = 1;
	intr->IMOI = 0;
	intr->IMOC = 0;

	// Configure the Event Ring Segment Table
	volatile XHCI_EVENT_RING_SEGMENT *erst = (XHCI_EVENT_RING_SEGMENT *) core_mapping(dcbaAddr + 0x800);
	erst->RSBA = physical_address((QWORD) device->event.RING);
	erst->RSSZ = 0x1000 / sizeof(XHCI_TRB_GENERIC);
	erst->RSV0 = 0;
	erst->RSV1 = 0;
	intr->STSZ = 1;
	intr->STBA = physical_address((QWORD) erst);

	// W1C Event Handler Busy bit
	intr->EHBS = 1;

	// Initialize and set ERDP
	intr->ERDP = physical_address((QWORD) device->event.RING) >> 4;

	// Clear and pending interrupts for the primary interrupter
	xhci_interrupt_ack(device, 0);
	/* ==== Runtime ==== */
}
void xhci_interrupt_ack(PCI_EXPRESS_XHCI_CONTROLLER *device, BYTE intx)
{
	// Clear the EINT bit in USBSYS by writting '1' to it
	device->operational->USTS = XHCI_USBSTS_EINT;

	// Get the interrupter registers
	volatile XHCI_INTERRUPTER *intr = device->runtime->INTR + intx;

	// Set the IP bit to '1' to clear it, preserve other bits including IE
	intr->IPEN = 1;
}
DWORD xhci_send_command(PCI_EXPRESS_XHCI_CONTROLLER *device, void *trb, XHCI_TRB_COMMAND_COMPLETION *completion)
{
	xhc_queue_transfer(&device->command, trb);
	volatile XHCI_TRB_COMMAND_COMPLETION *comp = &device->command.COMP;
	comp->CCOD = 0;
	xhc_command_doorbell(device->doorbell);
	while (!comp->CCOD) delay(1);
	if (completion)
		__memcpy(completion, (void *) comp, sizeof(XHCI_TRB_COMMAND_COMPLETION));
	return comp->CCOD;
}
void xhci_disable_slot(PCI_EXPRESS_XHCI_CONTROLLER *device, DWORD slotId)
{
	printf("Disable slot: %lu\n", slotId);
	if (!slotId)
		return;
	if (slotId > device->capability->SLOT)
		return;
	QWORD outputContext = device->context[slotId];
	if (outputContext)
		free_physical_memory(outputContext, 1);
	device->context[slotId] = 0;
	XHCI_TRB_DISABLE_SLOT trb;
	__memset(&trb, 0, sizeof(XHCI_TRB_DISABLE_SLOT));
	trb.TYPE = XHCI_TRB_TYPE_DISABLE_SLOT;
	trb.SLOT = slotId;
	DWORD cc;
	if ((cc = xhci_send_command(device, &trb, 0)) != XHCI_CODE_SUCCESS)
		printf("Disable slot command failed: %lu\n", cc);
}
void xhc_event_ring_process(PCI_EXPRESS_XHCI_CONTROLLER *device)
{
	XHCI_TRANSFER_RING *ring = &device->event;
	while (1)
	{
		XHCI_TRB_GENERIC *blk = xhc_event_ring_pop(ring);
		if (!blk)
			break;
		// Process Event
		BYTE type = (blk->CTRL >> 10) & 0x3F;
		if (type == XHCI_TRB_TYPE_TRANSFER_EVENT)
		{
			XHCI_TRB_TRANSFER_EVENT *xfer = (XHCI_TRB_TRANSFER_EVENT *) blk;
			continue;
		}
		if (type == XHCI_TRB_TYPE_COMMAND_COMPLETION)
		{
			__memcpy(&device->command.COMP, blk, sizeof(XHCI_TRB_COMMAND_COMPLETION));
			continue;
		}
		if (type == XHCI_TRB_TYPE_PORT_STATUS_CHANGE)
		{
			XHCI_TRB_PORT_STATUS_CHANGE *trb = (XHCI_TRB_PORT_STATUS_CHANGE *) blk;
			DWORD portId = trb->PRID - 1;
			volatile XHCI_PORT_SPACE *port = device->operational->PORT + portId;
			XHCI_PORT_STATUS *status = (XHCI_PORT_STATUS *) device->status + portId;
			printf("Port %lu Status Change: %u\n", portId, trb->CCOD);
			if (port->CSCH)
				printf("Connection Status: %u\n", port->CCSS);
			if (port->PECH)
				printf("Port Enable: %u\n", port->POEN);
			if (port->WRCH)
			{
				status->RST = 1;
				simple_output("Warm Reset complete\n");
			}
			if (port->OCCH)
				printf("Over-current condition: %u\n", port->OCAC);
			if (port->PRCH)
			{
				status->RST = 1;
				simple_output("Reset complete\n");
			}
			if (port->PLCH)
				printf("Port Link Status: %u\n", port->PLST);
			if (port->CECH)
			{
				status->ERR = 1;
				simple_output("Port Config Error\n");
			}
			// Clear port status change bits
			xhci_port_ack_port_changes(device, portId, XHCI_PORTSC_PSC_MASK);
			continue;
		}
		printf("TRB Type %u\n", type);
	}

	// Update ERDP
	//xhc_event_ring_update_dequeue(ring);
	device->runtime->INTR->ERDP = physical_address((QWORD) (ring->RING + ring->INDX)) >> 4;

	// Clear the Event Handle Busy bit
	device->runtime->INTR->EHBS = 1;
}
void xhci_interrupt(INTERRUPT_STACK *stack)
{
	xhc_event_ring_process(&DEVICE);

	xhci_interrupt_ack(&DEVICE, 0);
	eoi_apic(0);
}
void xhci_setup_device(PCI_EXPRESS_XHCI_CONTROLLER *device, DWORD portId)
{
	if (xhci_port_reset(device, portId))
	{
		simple_output("Reset port failed\n");
		return;
	}

	XHCI_TRB_ENABLE_SLOT trb;
	__memset(&trb, 0, sizeof(XHCI_TRB_ENABLE_SLOT));
	trb.TYPE = XHCI_TRB_TYPE_ENABLE_SLOT;
	XHCI_TRB_COMMAND_COMPLETION completion;
	xhci_send_command(device, &trb, &completion);
	if (completion.CCOD != XHCI_CODE_SUCCESS)
	{
		simple_output("Enable slot failed: ");
		simple_output_number(completion.CCOD);
		outchar('\n');
		return;
	}
	DWORD slotId = completion.SLID;

	USB_DEVICE.controller = (void *) device;
	if (xhci_setup_usb_device((XHCI_USB_DEVICE *) &USB_DEVICE, portId, slotId))
	{
		simple_output("Setup device failed\n");
		xhci_disable_slot(device, slotId);
	}
	USB_DEVICE.root = portId;

	// Enumerate device
	xhci_usb_enumerate_device((XHCI_USB_DEVICE *) &USB_DEVICE);
}