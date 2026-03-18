#pragma once
#include <intrinsic.h>
#include <pcie.h>

typedef union _PCI_DEVICE_VENDOR
{
	struct
	{
		WORD VENDOR;
		WORD DEVICE;
	};
	DWORD ID;
} PCI_DEVICE_VENDOR;

void *pci_enable_mmio(DWORD device, DWORD addr);
void *pcie_enable_mmio(volatile PCI_CONFIGURATION_SPACE *conf, DWORD addr);