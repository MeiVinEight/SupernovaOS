#include <driver/usb/xhci/xhci.h>
#include <console.h>
#include <core.h>
#include <intrinsic.h>

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

	DWORD reset = xhci_reset_controller(&device);
	if (!reset)
	{
		simple_output("Reset failed\n");
		return;
	}

	simple_output("Successful reset\n");
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
	{
		simple_output("Command Register not clear after reset: ");
		simple_output_address(xhci_operational_command(device), 8);
		outchar('\n');
		return 0;
	}

	if (device->operational->DNCR)
	{
		simple_output("Device Notification Controller not clear after reset: ");
		simple_output_address(device->operational->DNCR, 8);
		outchar('\n');
		return 0;
	}

	if (device->operational->CRCR)
	{
		simple_output("Context Ring Controller Register not clear after reset: ");
		simple_output_address(device->operational->CRCR, 16);
		outchar('\n');
		return 0;
	}

	if (device->operational->CBAA)
	{
		simple_output("Context Base Address Array not clear after reset: ");
		simple_output_address(device->operational->CBAA, 16);
		outchar('\n');
		return 0;
	}

	if (xhci_operational_config(device))
	{
		simple_output("Configuration not clear after reset: ");
		simple_output_address(xhci_operational_config(device), 8);
		outchar('\n');
		return 0;
	}

	return 1;
}