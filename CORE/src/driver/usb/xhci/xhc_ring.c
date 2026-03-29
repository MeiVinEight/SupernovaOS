#include <driver/usb/xhci/xhc_ring.h>
#include <core.h>
#include <memory/virtmem.h>
#include <intrinsic.h>

void xhc_command_ring_create(volatile XHCI_COMMAND_RING *ring)
{
	QWORD pc = 1;
	ring->RING = (XHCI_TRB_GENERIC *) core_mapping(alloc_physical_memory(&pc, 0, 0));
	__memset(ring->RING, 0, 0x1000);
	ring->INDX = 0;
	ring->CYCL = 1;
	ring->RING[0xFF].DATA = ((QWORD) ring->RING) & 0x0000007FFFFFFFFFULL;
	ring->RING[0xFF].CTRL = 2 | (XHCI_TRB_TYPE_LINK << 10);
}
void xhc_queue_command(volatile XHCI_COMMAND_RING *ring, void *trb)
{
	if (ring->INDX == 0xFF)
	{
		ring->RING[0xFF].CTRL ^= 1;
		ring->CYCL ^= 1;
		ring->INDX = 0;
	}
	XHCI_TRB_GENERIC *blk = trb;
	blk->CTRL &= ~1;
	blk->CTRL |= ring->CYCL;
	ring->RING[ring->INDX++] = *blk;
}
void xhc_event_ring_create(volatile XHCI_EVENT_RING *ring, volatile XHCI_INTERRUPTER *interrupter)
{
	ring->INTE = interrupter;
	ring->INDX = 0;
	ring->CYCL = 1;

	// Create the event ring segment memory block
	QWORD pc = 1;
	QWORD erdp = alloc_physical_memory(&pc, 0, 0);
	ring->RING = (XHCI_TRB_GENERIC *) core_mapping(erdp);
	__memset(ring->RING, 0, pc << 12);

	// Create the event ring segment table
	QWORD erstba = alloc_physical_memory(&pc, 0, 0);
	ring->ERST = (XHCI_EVENT_RING_SEGMENT *) core_mapping(erstba);
	__memset(ring->ERST, 0, pc << 12);

	// Construct the segment table entry
	ring->ERST->RSBA = erdp;
	ring->ERST->RSSZ = 0x1000 / sizeof(XHCI_TRB_GENERIC);
	ring->ERST->RSV0 = 0;
	ring->ERST->RSV1 = 0;

	// Configure the Event Ring Segment Table Size (ERSTSZ) register
	ring->INTE->STSZ = 1;

	// Initialize and set ERDP
	xhc_event_ring_update_dequeue(ring);

	// Write to ERSTBA register
	interrupter->STBA = erstba;

}
void xhc_event_ring_update_dequeue(volatile XHCI_EVENT_RING *ring)
{
	ring->INTE->ERDP = physical_address((QWORD) (ring->RING + ring->INDX)) >> 4;
}
XHCI_TRB_GENERIC *xhc_event_ring_pop(volatile XHCI_EVENT_RING *ring)
{
	if ((ring->RING[ring->INDX].CTRL & XHCI_TRB_CTRL_CYCLE) != ring->CYCL)
		return 0;

	XHCI_TRB_GENERIC *ret = (XHCI_TRB_GENERIC *) ring->RING + ring->INDX;

	++ring->INDX;
	if (!ring->INDX)
		ring->CYCL ^= 1;
	return ret;
}
void xhc_event_ring_process(volatile XHCI_EVENT_RING *ring)
{
	while (1)
	{
		volatile XHCI_TRB_GENERIC *blk = xhc_event_ring_pop(ring);
		if (!blk)
			break;
		// Process Event
	}

	// Update ERDP
	xhc_event_ring_update_dequeue(ring);

	// Clear the Event Handle Busy bit
	ring->INTE->EHBS = 1;
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