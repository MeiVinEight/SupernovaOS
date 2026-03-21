//
// Created by MeiVi on 2026/03/21.
//
#ifndef SUPERNOVA_XHC_RING_H
#define SUPERNOVA_XHC_RING_H

#include <types.h>
#include <driver/usb/xhci/xhc_trb.h>

typedef struct _XHCI_TRANSFER_RING
{
	XHCI_TRB_GENERIC *RING;
	BYTE INDX;
	BYTE CYCL;
} XHCI_TRANSFER_RING;

void xhc_ring_create(XHCI_TRANSFER_RING *ring);

#endif //SUPERNOVA_XHC_RING_H
