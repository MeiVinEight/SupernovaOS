//
// Created by MeiVi on 2026/03/21.
//

#ifndef SUPERNOVA_XHC_TRB_H
#define SUPERNOVA_XHC_TRB_H

#include <types.h>

#define XHCI_TRB_TYPE_LINK 6

typedef struct _XHCI_TRB_GENERIC
{
	QWORD DATA;
	DWORD STAT;
	DWORD CTRL;
} XHCI_TRB_GENERIC;
typedef struct _XHCI_TRB_NORMAL
{
	QWORD DATA;
	DWORD TRBL:17; // TRB Transfer Length
	DWORD TDSZ:5; // TD Size
	DWORD INTT:10; // Interrupter Target
	DWORD CYCL:1; // Cycle bit
	DWORD NEXT:1; // Evaluate Next TRB
	DWORD ISPK:1; // Interrupt-on Short Packet
	DWORD NOSN:1; // No Snoop
	DWORD CHAN:1; // Chain bit
	DWORD IONC:1; // Interrupt On Completion
	DWORD IMMD:1; // Immediate Data
	DWORD RSV0:2;
	DWORD BEIT:1; // Block Event Interrupt
	DWORD TYPE:6; // TRB Type
	DWORD RSV1:16;
} XHCI_TRB_NORMAL;

/* ==== Other TRBs ====*/

typedef struct _XHCI_TRB_LINK
{
	QWORD RING; // Ring Segment Pointer, should on a 16-byte boundry
	DWORD RSV0:22;
	DWORD INTT:10; // Interrupter Target
	DWORD CYCL:1; // Cycle bit
	DWORD CYCT:1; // Cycle Toggle
	DWORD RSV1:2;
	DWORD CHAN:1; // Chain bit
	DWORD IONC:1; // Interrupt On Completion
	DWORD RSV2:4;
	DWORD TYPE:6; // TRB Type
	DWORD RSV3:16;
} XHCI_TRB_LINK;

#endif //SUPERNOVA_XHC_TRB_H
