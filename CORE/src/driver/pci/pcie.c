#include <driver/pci/pcie.h>
#include <intrinsic.h>
#include <core.h>
#include <driver/pci/pci.h>
#include <driver/usb/xhci/xhci.h>
#include <console.h>

COREAPI volatile ACPI_MCFG *volatile MCFG = 0;

void setup_pcie_mcfg(ACPI_MCFG *mcfg)
{
	simple_output("MCFG @ ");
	simple_output_address((QWORD) mcfg, 16);
	outchar('\n');
	MCFG = mcfg;
}
void setup_pcie()
{
	if (!MCFG)
		return;

	QWORD length = MCFG->HEAD.LENG;
	DWORD entryCount = (length - /*sizeof(ACPI_MCFG)*/ 44) / sizeof(PCIE_SEGMENT_ADDRESS);
	for (volatile DWORD entryIdx = 0; entryIdx < entryCount; entryIdx++)
	{
		volatile QWORD segAddr = core_mapping(MCFG->ECAM[entryIdx].ECAM);

		for (volatile BYTE bus = 0; (bus + MCFG->ECAM[entryIdx].SBUS) < MCFG->ECAM[entryIdx].EBUS; bus++)
		{
			// Enumerate BUS
			volatile QWORD busAddr = segAddr + ((QWORD) bus << 20);

			for (volatile BYTE device = 0; device < 32; device++)
			{
				// Enumerate DEVICE
				volatile QWORD devAddr = busAddr + ((QWORD) device << 15);
				for (volatile BYTE func = 0; func < 8; func++)
				{
					// Enumerate function
					volatile QWORD funcAddr = devAddr + ((QWORD) func << 12);
					volatile PCI_CONFIGURATION_SPACE *conf = (PCI_CONFIGURATION_SPACE *) funcAddr;
					if ((!conf->device) || (conf->device == 0xFFFF))
						continue;

					volatile PCI_DEVICE_VENDOR vendor;
					vendor.VENDOR = conf->vendor;
					vendor.DEVICE = conf->device;
					simple_output("PCI @ ");
					simple_output_address(funcAddr, 16);
					simple_output(" - ");
					simple_output_address(conf->subsystem, 8);
					simple_output(": ");
					simple_output_address(conf->class, 6);
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

					volatile PCIE_DEVICE pcie;
					pcie.bus = bus;
					pcie.device = device;
					pcie.function = func;
					pcie.configuration = conf;
					if (conf->class == 0x0C0330) // xHCI
						setup_usb_xhci_pcie(&pcie);
				}
			}
		}
	}
}
QWORD pcie_cfg_get_base_address(volatile PCIE_DEVICE *device, DWORD addrIdx)
{
	QWORD bar = device->configuration->address[addrIdx];
	DWORD isIo = bar & PCI_BAR_IO_SPACE;
	if (isIo)
	{
		DWORD original = bar;
		bar &= 0xFFFFFFFC;

		/*
		volatile PCI_DEVICE_ADDRESS addr;
		addr.BUS = device->bus;
		addr.DVC = device->device;
		addr.FUN = device->function;
		addr.ENABLE = 1;
		// Determine size by writing all ones, reading back, and restoring
		pci_write_config_dword(addr.address, PCI_OFFSET_BAR0 + addrIdx * 4, 0xFFFFFFFF);
		DWORD sizeRead = pci_read_config_dword(addr.address, PCI_OFFSET_BAR0 + addrIdx * 4);

		// Restore original value
		pci_write_config_dword(addr.address, PCI_OFFSET_BAR0 + addrIdx * 4, 0xFFFFFFFF);

		sizeRead &= 0xFFFFFFFC;
		QWORD sizeVal = (~((QWORD) sizeRead) + 1);
		*/

		return bar;
	}
	DWORD type = ((bar >> 1) & 3);
	bar &= 0xFFFFFFF0;
	if (type == 0)
		return bar;
	if (type == 2)
		return (bar | ((QWORD) (device->configuration->address[addrIdx + 1]) << 32));

	return 0;
}