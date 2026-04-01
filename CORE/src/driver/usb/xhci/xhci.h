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

typedef struct _PCI_EXPRESS_XHCI_DEVICE
{
	PCI_EXPRESS_DEVICE pcie;
	QWORD address;
	volatile XHCI_CAPABILITY_SPACE *capability;
	volatile XHCI_OPERATIONAL_SPACE *operational;
	volatile XHCI_RUNTIME_SPACE *runtime;
	volatile XHCI_DOORBELL *doorbell;
	volatile QWORD *context;
	XHCI_COMMAND_RING command;
	XHCI_EVENT_RING event;
	WORD status[32];
	BYTE slot[32];
} PCI_EXPRESS_XHCI_DEVICE;

void setup_usb_xhci_pcie(volatile PCI_EXPRESS_DEVICE *device);
QWORD xhci_get_scratchpad_buffer(volatile PCI_EXPRESS_XHCI_DEVICE *device);
DWORD xhci_operational_command(volatile PCI_EXPRESS_XHCI_DEVICE *device);
DWORD xhci_operational_status(volatile PCI_EXPRESS_XHCI_DEVICE *device);
DWORD xhci_operational_config(volatile PCI_EXPRESS_XHCI_DEVICE *device);
DWORD xhci_reset_controller(volatile PCI_EXPRESS_XHCI_DEVICE *device);
DWORD xhci_start_controller(volatile PCI_EXPRESS_XHCI_DEVICE *device);
void xhci_configure_controller(volatile PCI_EXPRESS_XHCI_DEVICE *device);
void xhci_interrupt_ack(volatile PCI_EXPRESS_XHCI_DEVICE *device, BYTE intr);
void xhci_interrupt(INTERRUPT_STACK *stack);

#endif //SUPERNOVA_XHCI_H
