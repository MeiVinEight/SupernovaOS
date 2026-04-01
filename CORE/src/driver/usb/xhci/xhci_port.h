//
// Created by MeiVi on 2026/03/31.
//

#ifndef SUPERNOVAOS_XHCI_PORT_H
#define SUPERNOVAOS_XHCI_PORT_H

#include <driver/usb/xhci/xhci.h>

#define XHCI_PORTSC_RWX_MASK 0x0E01C3E0
#define XHCI_PORTSC_W1C_MASK 0x00FE0002
#define XHCI_PORTSC_PSC_MASK 0x00FE0000
#define XHCI_PORTSC_PPWR     (1 << 9)
#define XHCI_PORTSC_PRST     (1 << 4)
#define XHCI_PORTSC_WRST     (1 << 31)

typedef struct _XHCI_PORT_STATUS
{
	// Set to 1 when port reset complete
	WORD RST:1;
	// Set to 1 when any error occurred (Port Config Error Change asserted)
	WORD ERR:1;
	WORD RSV:14;
} XHCI_PORT_STATUS;

DWORD xhci_port_reset(volatile PCI_EXPRESS_XHCI_DEVICE *device, DWORD portId);
void xhci_port_ack_port_changes(volatile PCI_EXPRESS_XHCI_DEVICE *device, DWORD portId, DWORD changeMask);

#endif //SUPERNOVAOS_XHCI_PORT_H
