//
// Created by MeiVi on 2026/03/21.
//
#ifndef SUPERNOVA_XHC_RING_H
#define SUPERNOVA_XHC_RING_H

#include <types.h>
#include <driver/usb/xhci/xhc_trb.h>
#include <driver/usb/xhci/xhc_regs.h>

#define XHCI_DOORBELL_COMMAND_RING 0
#define XHCI_DOORBELL_CONTROL_RING 1

typedef struct _XHCI_TRANSFER_RING
{
	XHCI_TRB_GENERIC *RING;
	BYTE INDX;
	BYTE CYCL;
} XHCI_TRANSFER_RING;

XHCI_TRB_GENERIC *xhc_event_ring_pop(XHCI_TRANSFER_RING *ring);
// Target = 2 + (2 * ZeroBasedEndpoint) + (IsOutEP ? 0 : 1)
void xhc_ring_doorbell(XHCI_DOORBELL *doorbell, BYTE id, BYTE target);
void xhc_command_doorbell(XHCI_DOORBELL *doorbell);
void xhc_control_doorbell(XHCI_DOORBELL *doorbell, DWORD id);
void xhc_transfer_ring_create(XHCI_TRANSFER_RING *ring, int link);
void *xhc_queue_transfer(XHCI_TRANSFER_RING *ring, void *trb);

#endif //SUPERNOVA_XHC_RING_H
