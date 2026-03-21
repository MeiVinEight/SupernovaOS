#include <driver/usb/xhci/xhc_ring.h>
#include <core.h>
#include <memory/virtmem.h>
#include <intrinsic.h>

void xhc_ring_create(XHCI_TRANSFER_RING *ring)
{
	QWORD pc = 1;
	ring->RING = (XHCI_TRB_GENERIC *) core_mapping(alloc_physical_memory(&pc, 0, 0));
	__memset(ring->RING, 0, 0x1000);
	ring->CYCL = 1;
	ring->INDX = 0;
	ring->RING[0xFF].DATA = ((QWORD) ring->RING) & 0x0000007FFFFFFFFFULL;
	ring->RING[0xFF].CTRL = 2 | (XHCI_TRB_TYPE_LINK << 10);
}
void xhc_ring_queue(XHCI_TRANSFER_RING *ring, void *trb)
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