#pragma once

#include <types.h>
#include <driver/pci/pcie.h>

typedef struct _NVM_EXPRESS_CONTROLLER NVM_EXPRESS_CONTROLLER;
struct _NVM_EXPRESS_CONTROLLER
{
	PCI_EXPRESS_DEVICE      PCIE;
	NVM_EXPRESS_CONTROLLER *NEXT;
};

void nvme_controller_setup(PCI_EXPRESS_DEVICE *pcie);