//
// Created by MeiVi on 2026/03/18.
//

#ifndef SUPERNOVA_XHCI_H
#define SUPERNOVA_XHCI_H

#include <types.h>
#include <driver/pci/pcie.h>
#include <driver/usb/xhci/xhc_regs.h>
#include <driver/usb/xhci/xhc_ring.h>
#include <interrupt/interrupt.h>

typedef struct _PCI_EXPRESS_XHCI_CONTROLLER
{
	PCI_EXPRESS_DEVICE pcie;
	QWORD address;
	volatile XHCI_CAPABILITY_SPACE *capability;
	volatile XHCI_OPERATIONAL_SPACE *operational;
	volatile XHCI_RUNTIME_SPACE *runtime;
	volatile XHCI_DOORBELL *doorbell;
	volatile QWORD *context;
	XHCI_TRANSFER_RING command;
	XHCI_TRANSFER_RING event;
	WORD status[32];
} PCI_EXPRESS_XHCI_CONTROLLER;

void setup_usb_xhci_pcie(PCI_EXPRESS_DEVICE *device);
DWORD xhci_reset_controller(PCI_EXPRESS_XHCI_CONTROLLER *device);
DWORD xhci_start_controller(PCI_EXPRESS_XHCI_CONTROLLER *device);
void xhci_configure_controller(PCI_EXPRESS_XHCI_CONTROLLER *device);
void xhci_interrupt_ack(PCI_EXPRESS_XHCI_CONTROLLER *device, BYTE intr);
DWORD xhci_send_command(PCI_EXPRESS_XHCI_CONTROLLER *device, void *trb, XHCI_TRB_COMMAND_COMPLETION *completion);
void xhci_interrupt(INTERRUPT_STACK *stack);
void xhci_setup_device(PCI_EXPRESS_XHCI_CONTROLLER *device, DWORD portId);

#endif //SUPERNOVA_XHCI_H
