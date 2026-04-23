#include <driver/nvme/nvme.h>
#include <mm/vmm.h>
#include <driver/pci/pci.h>
#include <core.h>
#include <stdio.h>

NVM_EXPRESS_CONTROLLER *NVME_CONTROLLER;

void nvme_controller_setup(PCI_EXPRESS_DEVICE *pcie)
{
	PCI_DEVICE_VENDOR vendor;
	vendor.VENDOR = pcie->configuration->vendor;
	vendor.DEVICE = pcie->configuration->device;
	printf("PCI Express @ %016llX: %06X - ", (QWORD) pcie->configuration, pcie->configuration->class);
	const char *vendorName = pci_vendor_name(vendor.VENDOR);
	const char *deviceName = pci_device_name(vendor);
	if (vendorName && deviceName)
		printf("%s %s\n", vendorName, deviceName);
	else
		printf("%08lX\n", vendor.ID);

	NVM_EXPRESS_CONTROLLER *controller = heap_alloc(sizeof(NVM_EXPRESS_CONTROLLER));
	controller->PCIE = *pcie;
	controller->NEXT = NVME_CONTROLLER;
	NVME_CONTROLLER = controller;
	controller->NVME = (NVM_EXPRESS_CONTROLLER_SPACE *) core_mapping(pcie_cfg_get_base_address(&controller->PCIE,  0));
	printf("NVM Express @ %p\n", controller->NVME);
	printf("Submission Queue: %p\n", (void *) controller->NVME->ASQA);
	printf("Completion Queue: %p\n", (void *) controller->NVME->ACQA);
}