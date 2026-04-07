#include <driver/pci/pcie.h>
#include <intrinsic.h>
#include <core.h>
#include <driver/pci/pci.h>
#include <driver/xhci/xhci.h>
#include <driver/pci/msi/msix.h>
#include <driver/pci/msi/msi.h>
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
	for (DWORD entryIdx = 0; entryIdx < entryCount; entryIdx++)
	{
		QWORD segAddr = core_mapping(MCFG->ECAM[entryIdx].ECAM);

		for (DWORD bus = 0; (bus + MCFG->ECAM[entryIdx].SBUS) <= MCFG->ECAM[entryIdx].EBUS; bus++)
		{
			// Enumerate BUS
			QWORD busAddr = segAddr + ((QWORD) bus << 20);

			for (DWORD device = 0; device < 32; device++)
			{
				// Enumerate DEVICE
				QWORD devAddr = busAddr + ((QWORD) device << 15);
				for (WORD func = 0; func < 8; func++)
				{
					// Enumerate function
					QWORD funcAddr = devAddr + ((QWORD) func << 12);
					PCI_CONFIGURATION_SPACE *conf = (PCI_CONFIGURATION_SPACE *) funcAddr;
					if ((!conf->device) || (conf->device == 0xFFFF))
						continue;

					PCI_DEVICE_VENDOR vendor;
					vendor.VENDOR = conf->vendor;
					vendor.DEVICE = conf->device;

					/*
					printf("PCI Express (%02lX.%02lX.%X) @ %016llX: %06X - ", bus, device, func, funcAddr, conf->class);
					const char *vendorName = pci_vendor_name(vendor.VENDOR);
					const char *deviceName = pci_device_name(vendor);
					if (vendorName && deviceName)
						printf("%s %s", vendorName, deviceName);
					else
						simple_output_address(vendor.ID, 8);
					outchar('\n');
					*/

					PCI_EXPRESS_DEVICE pcie;
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
QWORD pcie_cfg_get_base_address(PCI_EXPRESS_DEVICE *device, DWORD addrIdx)
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
DWORD pcie_bar_cound(PCI_EXPRESS_DEVICE *device)
{
	if (device->configuration->type & 0x7F)
		return 2;
	return 6;
}
PCI_EXPRESS_CAPABILITY *__stdcall pcie_capability(PCI_EXPRESS_DEVICE *device, DWORD capa)
{
	BYTE capId = capa;
	QWORD baseAddr = (QWORD) device->configuration;
	/*
	The bottom two bits are Reserved and must be set to 00b. Software must mask these bits
	off before using this register as a pointer in Configuration Space to the first entry of a linked
	list of new capabilities.
	*/
	BYTE capaOff = device->configuration->capability & 0xFC;
	DWORD maxCapCount = 48;
	while (capaOff && maxCapCount--)
	{
		volatile PCI_EXPRESS_CAPABILITY *cap = (PCI_EXPRESS_CAPABILITY *) (baseAddr + capaOff);
		if (cap->CAID == capId)
			return (PCI_EXPRESS_CAPABILITY *) cap;
		capaOff = cap->NEXT;
	}
	return 0;
}
DWORD pcie_setup_interrupt(PCI_EXPRESS_DEVICE *device, void (*irq)(INTERRUPT_STACK *stack), BYTE intx)
{
	if (pcie_setup_msix(device, intx))
	{
		register_interrupt(intx, irq);
		return 1;
	}
	if (pcie_setup_msi(device, intx))
	{
		register_interrupt(intx, irq);
		return 1;
	}
	if (device->configuration->interrupt.line && (~device->configuration->interrupt.line))
	{
	}
	return 0;
}