#include <driver/usb/xhci/xhci.h>
#include <console.h>
#include <core.h>
#include <intrinsic.h>
#include <memory/virtmem.h>
#include <driver/usb/xhci/xhc_regs.h>
#include <driver/pci/pci.h>
#include <interrupt/apic.h>
#include <arch/processor.h>

COREAPI volatile PCI_EXPRESS_XHCI_DEVICE DEVICE;

void setup_usb_xhci_pcie(volatile PCI_EXPRESS_DEVICE *dev)
{
	volatile PCI_DEVICE_VENDOR vendor;
	vendor.VENDOR = dev->configuration->vendor;
	vendor.DEVICE = dev->configuration->device;
	simple_output("PCI @ ");
	simple_output_address((QWORD) dev->configuration, 16);
	simple_output(" - ");
	simple_output_address(dev->configuration->subsystem, 8);
	simple_output(": ");
	simple_output_address(dev->configuration->class, 6);
	simple_output(" - ");
	const char *vendorName = pci_vendor_name(vendor.VENDOR);
	const char *deviceName = pci_device_name(vendor);
	if (vendorName && deviceName)
	{
		simple_output(vendorName);
		outchar(' ');
		simple_output(deviceName);
	}
	else
		simple_output_address(vendor.ID, 8);
	outchar('\n');

	pcie_setup_interrupt(dev, xhci_interrupt, interrupt_alloc_intx());

	//device.pcie = *dev;
	__memcpy(&DEVICE.pcie, (void *) dev, sizeof(PCI_EXPRESS_DEVICE));

	QWORD xhciBase = pcie_cfg_get_base_address(dev, 0);
	DEVICE.capability = (XHCI_CAPABILITY_SPACE *) core_mapping(xhciBase);
	DEVICE.operational = (XHCI_OPERATIONAL_SPACE *) core_mapping(xhciBase + DEVICE.capability->SIZE);
	DEVICE.runtime = (XHCI_RUNTIME_SPACE *) core_mapping(xhciBase + DEVICE.capability->RTME);
	DEVICE.doorbell = (XHCI_DOORBELL *) core_mapping(xhciBase + DEVICE.capability->BELL);

	simple_output("==== xHCI Capability Address ");
	simple_output_address((QWORD) DEVICE.capability, 16);
	simple_output(" ====\n");
	simple_output("    Length: ");
	simple_output_number(DEVICE.capability->SIZE);
	outchar('\n');
	simple_output("    Max Device Slots: ");
	simple_output_number(DEVICE.capability->SLOT);
	outchar('\n');
	simple_output("    Max Interrupters: ");
	simple_output_number(DEVICE.capability->INTE);
	outchar('\n');
	simple_output("    Max Ports: ");
	simple_output_number(DEVICE.capability->PORT);
	outchar('\n');

	simple_output("    IST: ");
	simple_output_number(DEVICE.capability->ISTH);
	outchar('\n');
	simple_output("    ERST Max Size: ");
	simple_output_number(DEVICE.capability->ERST);
	outchar('\n');
	simple_output("    Scratchpad Buffers: ");
	simple_output_address(xhci_get_scratchpad_buffer(&DEVICE), 8);
	outchar('\n');

	simple_output("    64-bit Addressing: ");
	simple_output(DEVICE.capability->AC64 ? "yes\n" : "no\n");
	simple_output("    Bandwidth Negotiation: ");
	simple_output_number(DEVICE.capability->BWNC);
	outchar('\n');
	simple_output("    64-Byte Context Size: ");
	simple_output(DEVICE.capability->CX64 ? "yes\n" : "no\n");
	simple_output("    Port Power Control: ");
	simple_output_number(DEVICE.capability->PPWC);
	outchar('\n');
	simple_output("    Port Indicators: ");
	simple_output_number(DEVICE.capability->PIND);
	outchar('\n');
	simple_output("    Light HC Reset Available: ");
	simple_output_number(DEVICE.capability->LHCR);
	outchar('\n');
	simple_output("    xHCI Extended Capability: ");
	simple_output_number(DEVICE.capability->XECP);
	outchar('\n');

	DWORD reset = xhci_reset_controller(&DEVICE);
	if (!reset)
	{
		simple_output("Reset failed\n");
		return;
	}

	simple_output("Successful reset\n");

	xhci_configure_controller(&DEVICE);
	simple_output("==== xHCI Operational Address ");
	simple_output_address((QWORD) DEVICE.operational, 16);
	simple_output(" ====\n");

	simple_output("    command: ");
	simple_output_address(xhci_operational_command(&DEVICE), 8);
	outchar('\n');
	simple_output("    status: ");
	simple_output_address(xhci_operational_status(&DEVICE), 8);
	outchar('\n');
	simple_output("    pagesize: ");
	simple_output_address(DEVICE.operational->PAGE, 8);
	outchar('\n');
	simple_output("    notify: ");
	simple_output_address(DEVICE.operational->DNCR, 8);
	outchar('\n');
	simple_output("    Context Ring: ");
	simple_output_address(DEVICE.operational->CRCR, 16);
	outchar('\n');
	simple_output("    Context Base Address Array: ");
	simple_output_address(DEVICE.operational->CBAA, 16);
	outchar('\n');
	simple_output("    config: ");
	simple_output_address(xhci_operational_config(&DEVICE), 8);
	outchar('\n');

	if (!xhci_start_controller(&DEVICE))
	{
		simple_output("XHCI start failed\n");
		return;
	}
	__halt();
	__halt();
	__halt();
	__halt();
	__halt();
	__halt();
	__halt();
	__halt();
	__halt();
	__halt();
	simple_output("Controller started!\n");

	// Test xHCI Event
	XHCI_TRB_ENABLE_SLOT trb;
	__memset(&trb, 0, sizeof(XHCI_TRB_ENABLE_SLOT));
	trb.TYPE = XHCI_TRB_TYPE_ENABLE_SLOT;
	// Send the command twice, and it should trigger the interrupt twice.
	xhc_queue_command(&DEVICE.command, &trb);
	xhc_command_doorbell(DEVICE.doorbell);

	__halt();
	__halt();
	__halt();
	__halt();
	__halt();
	__halt();
	__halt();
	__halt();
	__halt();
	__halt();

	xhc_queue_command(&DEVICE.command, &trb);
	xhc_command_doorbell(DEVICE.doorbell);

	__halt();
	__halt();
	__halt();
	__halt();

	// Foreach xECP
	volatile QWORD xadr = xhciBase;
	volatile DWORD xecp = DEVICE.capability->XECP << 2;
	while (xecp)
	{
		xadr = xadr + xecp;
		volatile XHCI_EXTENDED_CAPABILITY *xcap = (XHCI_EXTENDED_CAPABILITY *) core_mapping(xadr);
		if (xcap->CAID == XHCI_XECP_SUPPORTED_PROTOCOL)
		{
			volatile XHCI_CAPABILITY_SUPPORTED_PROTOCOL *supp = (XHCI_CAPABILITY_SUPPORTED_PROTOCOL *) xcap;
			QWORD name = supp->NAME;
			simple_output(&name);
			simple_output_number(supp->MAJV);
			outchar('.');
			simple_output_number(supp->MINV);
			simple_output(": ");
			simple_output_number(supp->CPOF - 1);
			outchar('+');
			simple_output_number(supp->CPCN);
			outchar('\n');
		}
		xecp = xcap->NEXT << 2;
	}
}
QWORD xhci_get_scratchpad_buffer(volatile PCI_EXPRESS_XHCI_DEVICE *device)
{
	return (device->capability->MSBH << 5) | device->capability->MSBL;
}
DWORD xhci_operational_command(volatile PCI_EXPRESS_XHCI_DEVICE *device)
{
	return *((volatile DWORD *) ((QWORD) device->operational));
}
DWORD xhci_operational_status(volatile PCI_EXPRESS_XHCI_DEVICE *device)
{
	return *((volatile DWORD *) (((QWORD) device->operational) + 4));
}
DWORD xhci_operational_config(volatile PCI_EXPRESS_XHCI_DEVICE *device)
{
	return *((volatile DWORD *) (((QWORD) device->operational) + 0x38));
}
DWORD xhci_reset_controller(volatile PCI_EXPRESS_XHCI_DEVICE *device)
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
	if (xhci_operational_command(device))
		return 0;

	if (device->operational->DNCR)
		return 0;

	if (device->operational->CRCR)
		return 0;

	if (device->operational->CBAA)
		return 0;

	if (xhci_operational_config(device))
		return 0;

	return 1;
}
DWORD xhci_start_controller(volatile PCI_EXPRESS_XHCI_DEVICE *device)
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
void xhci_configure_controller(volatile PCI_EXPRESS_XHCI_DEVICE *device)
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
	DWORD maxScratchpadBuf = xhci_get_scratchpad_buffer(device);
	if (maxScratchpadBuf)
	{
		// TODO Check PAGESIZE in operational registers
		QWORD pcnt = maxScratchpadBuf << 3;
		pcnt += 0xFFF;
		pcnt >>= 12;
		QWORD scrArrAddr = alloc_physical_memory(&pcnt, 0, 0);
		volatile QWORD *scrpadArr = (QWORD *) core_mapping(scrArrAddr);
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
	xhc_command_ring_create((XHCI_COMMAND_RING *) &device->command);
	device->operational->CRCR = physical_address((QWORD) device->command.RING) | 1;
	/* ==== Operational ==== */


	/* ==== Runtime ==== */
	// Get the primary interrupter registers
	volatile XHCI_INTERRUPTER *intr = device->runtime->INTR;

	// Enable interrupts
	intr->IENA = 1;

	// Setup the event ring and write to interrupter
	// registers to se ERSTSZ, ERDP, and ERSTBA.
	xhc_event_ring_create(&device->event, intr);

	simple_output("    Event Ring Address: ");
	simple_output_address(intr->STBA, 16);
	outchar('\n');
	simple_output("    Event Ring Size: ");
	simple_output_number(intr->STSZ);
	outchar('\n');
	simple_output("    Dequeue Pointer: ");
	simple_output_address(intr->ERDP << 4, 16);
	outchar('\n');

	// Clear and pending interrupts for the primary interrupter
	xhci_interrupt_ack(device, 0);
	/* ==== Runtime ==== */

}
void xhci_interrupt_ack(volatile PCI_EXPRESS_XHCI_DEVICE *device, BYTE intx)
{
	// Clear the EINT bit in USBSYS by writting '1' to it
	device->operational->USTS = XHCI_USBSTS_EINT;

	// Get the interrupter registers
	volatile XHCI_INTERRUPTER *intr = device->runtime->INTR + intx;

	// Set the IP bit to '1' to clear it, preserve other bits including IE
	intr->IPEN = 1;
}
void xhci_interrupt(INTERRUPT_STACK *stack)
{
	simple_output("CPU #");
	simple_output_number(cpu_local_apic_id());
	simple_output(" INT: #");
	simple_output_address(stack->INT, 2);
	simple_output(" @ RIP ");
	simple_output_address(stack->RIP, 16);
	simple_output(": xHCI Message Event\n");

	xhc_event_ring_process(&DEVICE.event);

	xhci_interrupt_ack(&DEVICE, 0);
	eoi_apic(0);
}