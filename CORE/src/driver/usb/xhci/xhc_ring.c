#include <driver/usb/xhci/xhc_ring.h>
#include <core.h>
#include <memory/virtmem.h>
#include <intrinsic.h>

XHCI_TRB_GENERIC *xhc_event_ring_pop(volatile XHCI_TRANSFER_RING *ring)
{
	if ((ring->RING[ring->INDX].CTRL & XHCI_TRB_CTRL_CYCLE) != ring->CYCL)
		return 0;

	XHCI_TRB_GENERIC *ret = (XHCI_TRB_GENERIC *) ring->RING + ring->INDX;

	++ring->INDX;
	if (!ring->INDX)
		ring->CYCL ^= 1;
	return ret;
}
void xhc_ring_doorbell(volatile XHCI_DOORBELL *doorbell, BYTE id, BYTE target)
{
	volatile DWORD *raw = (DWORD *) doorbell;
	raw[id] = (DWORD) target;
}
void xhc_command_doorbell(volatile XHCI_DOORBELL *doorbell)
{
	xhc_ring_doorbell(doorbell, 0, XHCI_DOORBELL_COMMAND_RING);
}
void xhc_control_doorbell(volatile XHCI_DOORBELL *doorbell, DWORD id)
{
	xhc_ring_doorbell(doorbell, id, XHCI_DOORBELL_CONTROL_RING);
}
void xhc_transfer_ring_create(volatile XHCI_TRANSFER_RING *ring, int link)
{
	QWORD pc = 1;
	QWORD ringPhyAddr = alloc_physical_memory(&pc, 0, 0);
	ring->RING = (XHCI_TRB_GENERIC *) core_mapping(ringPhyAddr);
	ring->CYCL = 1;
	ring->INDX = 0;
	__memset(ring->RING, 0, pc << 12);
	if (link)
	{
		ring->RING[0xFF].DATA = ringPhyAddr;
		ring->RING[0xFF].CTRL = (XHCI_TRB_TYPE_LINK << 10) | 3;
	}
}
void *xhc_queue_transfer(XHCI_TRANSFER_RING *ring, void *trb)
{
	// Advance and possibly wrap the enqueue pointer if needed.
	if (ring->INDX == 0xFF)
	{
		ring->RING[0xFF].CTRL ^= 1;
		ring->CYCL ^= 1;
		ring->INDX = 0;
	}

	volatile XHCI_TRB_NORMAL *normal = (XHCI_TRB_NORMAL *) trb;
	// Adjust the TRB's cycle bit to the current DCS
	normal->CYCL = ring->CYCL;
	// Insert the TRB into the ring
	__memcpy(ring->RING + ring->INDX, (void *) normal, sizeof(XHCI_TRB_NORMAL));
	return ring->RING + ring->INDX++;
}