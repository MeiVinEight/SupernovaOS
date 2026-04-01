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

typedef struct _XHCI_COMMAND_RING
{
	XHCI_TRB_GENERIC *RING;
	BYTE INDX;
	BYTE CYCL;
} XHCI_COMMAND_RING;
typedef struct _XHCI_EVENT_RING
{
	volatile XHCI_INTERRUPTER *INTE;
	volatile XHCI_TRB_GENERIC *RING;
	volatile XHCI_EVENT_RING_SEGMENT *ERST;
	BYTE INDX;
	BYTE CYCL;
} XHCI_EVENT_RING;

void xhc_command_ring_create(volatile XHCI_COMMAND_RING *ring);
void *xhc_queue_command(volatile XHCI_COMMAND_RING *ring, void *trb);
void xhc_event_ring_create(volatile XHCI_EVENT_RING *ring, volatile XHCI_INTERRUPTER *interrupter);
void xhc_event_ring_update_dequeue(volatile XHCI_EVENT_RING *ring);
XHCI_TRB_GENERIC *xhc_event_ring_pop(volatile XHCI_EVENT_RING *ring);
// Target = 2 + (2 * ZeroBasedEndpoint) + (IsOutEP ? 0 : 1)
void xhc_ring_doorbell(volatile XHCI_DOORBELL *doorbell, BYTE id, BYTE target);
void xhc_command_doorbell(volatile XHCI_DOORBELL *doorbell);
void xhc_control_doorbell(volatile XHCI_DOORBELL *doorbell, DWORD id);

#endif //SUPERNOVA_XHC_RING_H
