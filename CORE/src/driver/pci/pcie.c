#include <driver/pci/pcie.h>
#include <intrinsic.h>
#include <core.h>
#include <driver/pci/pci.h>

#include "console.h"

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
				}
			}
		}
	}
}