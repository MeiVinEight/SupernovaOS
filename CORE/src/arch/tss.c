#include <arch/tss.h>
#include <intrinsic.h>
#include <mm/segment.h>

void load_tss(void *tss0, void *gdt0, WORD off)
{
	QWORD tssAddr = (QWORD) tss0;
	SYSTEM_SEGMENT_DESCRIPTOR *tssd = (SYSTEM_SEGMENT_DESCRIPTOR *) (((QWORD) gdt0) + off);
	tssd->LIMT = 0x67; // 103, 1 less than sizeof (tss)
	tssd->ADD0 = (tssAddr >>  0) & 0xFFFF;
	tssd->ADD1 = (tssAddr >> 16) & 0xFF;
	tssd->TYPE = 9; // TSS, Not Busy
	tssd->DPLX = 0; // Ring 0 can read/write
	tssd->PRST = 1; // Present
	tssd->AVAL = 1; // Available
	tssd->ADD2 = (tssAddr >> 24) & 0xFF;
	tssd->ADD3 = (tssAddr >> 32) & 0xFFFFFFFF;
	__ltr(off);
}