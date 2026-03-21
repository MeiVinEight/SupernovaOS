#include <driver/usb/xhci/xhci.h>
#include <console.h>
#include <core.h>
#include <intrinsic.h>
#include <memory/virtmem.h>

void setup_usb_xhci_pcie(volatile PCIE_DEVICE *dev)
{
	volatile PCIE_XHCI_DEVICE device;
	device.pcie = dev;

	QWORD xhciBase = pcie_cfg_get_base_address(dev, 0);
	device.capability = (XHCI_CAPABILITY_SPACE *) core_mapping(xhciBase);
	simple_output("==== xHCI Capability Address ");
	simple_output_address((QWORD) device.capability, 16);
	simple_output(" ====\n");
	simple_output("    Length: ");
	simple_output_number(device.capability->SIZE);
	outchar('\n');
	simple_output("    Max Device Slots: ");
	simple_output_number(device.capability->SLOT);
	outchar('\n');
	simple_output("    Max Interrupters: ");
	simple_output_number(device.capability->INTE);
	outchar('\n');
	simple_output("    Max Ports: ");
	simple_output_number(device.capability->PORT);
	outchar('\n');

	simple_output("    IST: ");
	simple_output_number(device.capability->ISTH);
	outchar('\n');
	simple_output("    ERST Max Size: ");
	simple_output_number(device.capability->ERST);
	outchar('\n');
	simple_output("    Scratchpad Buffers: ");
	simple_output_address(xhci_get_scratchpad_buffer(&device), 8);
	outchar('\n');

	simple_output("    64-bit Addressing: ");
	simple_output(device.capability->AC64 ? "yes\n" : "no\n");
	simple_output("    Bandwidth Negotiation: ");
	simple_output_number(device.capability->BWNC);
	outchar('\n');
	simple_output("    64-Byte Context Size: ");
	simple_output(device.capability->CX64 ? "yes\n" : "no\n");
	simple_output("    Port Power Control: ");
	simple_output_number(device.capability->PPWC);
	outchar('\n');
	simple_output("    Port Indicators: ");
	simple_output_number(device.capability->PIND);
	outchar('\n');
	simple_output("    Light HC Reset Available: ");
	simple_output_number(device.capability->LHCR);
	outchar('\n');
	simple_output("    xHCI Extended Capability: ");
	simple_output_number(device.capability->XECP);
	outchar('\n');

	device.operational = (volatile XHCI_OPERATIONAL_SPACE *) core_mapping(xhciBase + device.capability->SIZE);

	DWORD reset = xhci_reset_controller(&device);
	if (!reset)
	{
		simple_output("Reset failed\n");
		return;
	}

	simple_output("Successful reset\n");

	xhci_configure_operational(&device);
	simple_output("==== xHCI Operational Address ");
	simple_output_address((QWORD) device.operational, 16);
	simple_output(" ====\n");

	simple_output("    command: ");
	simple_output_address(xhci_operational_command(&device), 8);
	outchar('\n');
	simple_output("    status: ");
	simple_output_address(xhci_operational_status(&device), 8);
	outchar('\n');
	simple_output("    pagesize: ");
	simple_output_address(device.operational->PAGE, 8);
	outchar('\n');
	simple_output("    notify: ");
	simple_output_address(device.operational->DNCR, 8);
	outchar('\n');
	simple_output("    Context Ring: ");
	simple_output_address(device.operational->CRCR, 16);
	outchar('\n');
	simple_output("    Context Base Address Array: ");
	simple_output_address(device.operational->CBAA, 16);
	outchar('\n');
	simple_output("    config: ");
	simple_output_address(xhci_operational_config(&device), 8);
	outchar('\n');
}
QWORD xhci_get_scratchpad_buffer(volatile PCIE_XHCI_DEVICE *device)
{
	return (device->capability->MSBH << 5) | device->capability->MSBL;
}
DWORD xhci_operational_command(volatile PCIE_XHCI_DEVICE *device)
{
	return *((volatile DWORD *) ((QWORD) device->operational));
}
DWORD xhci_operational_status(volatile PCIE_XHCI_DEVICE *device)
{
	return *((volatile DWORD *) (((QWORD) device->operational) + 4));
}
DWORD xhci_operational_config(volatile PCIE_XHCI_DEVICE *device)
{
	return *((volatile DWORD *) (((QWORD) device->operational) + 0x38));
}
DWORD xhci_reset_controller(volatile PCIE_XHCI_DEVICE *device)
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
void xhci_configure_operational(volatile PCIE_XHCI_DEVICE *device)
{
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
		QWORD scrArrAddr = alloc_physical_memory(&pcnt, 0, 0);
		volatile QWORD *scrpadArr = (QWORD *) core_mapping(scrArrAddr);
		// Create scratchpad pages
		for (DWORD i = 0; i < maxScratchpadBuf; i++)
			scrpadArr[i] = alloc_physical_memory(&pcnt, 0, 0);

		// Set the first slot in the DCBAA to point to the scratchpad array
		device->context[0] = scrArrAddr;
	}

	// Set DCBAA Address into operational DCBAA registr
	device->operational->CBAA = dcbaAddr;

	// Setup command ring, CRCR
	xhc_ring_create((XHCI_TRANSFER_RING *) &device->CMMD);
	device->operational->CRCR = physical_address((QWORD) device->CMMD.RING) | 1;

}