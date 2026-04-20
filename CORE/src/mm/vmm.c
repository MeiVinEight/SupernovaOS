#include <mm/vmm.h>
#include <mm/pmm.h>
#include <core.h>
#include <intrinsic.h>
#include <interrupt/syscall.h>
#include <proc/proc.h>

#define MEMBLK_NODE_PRE_PAGE 0x7F

#define HEAP_FLAG_USING 1ULL
#define HEAP_FLAG_LAST  2ULL

typedef union _VIRTUAL_ADDRESS
{
	struct
	{
		QWORD offset0:12;
		QWORD offset1:9;
		QWORD offset2:9;
		QWORD offset3:9;
		QWORD offset4:9;
		QWORD reserve:16;
	};
	QWORD address;
} VIRTUAL_ADDRESS;

COREAPI LINEAR_MEMORY_BLOCK *volatile BLOCK_HEAP = 0;
COREAPI LINEAR_MEMORY_BLOCK *volatile MEMORY_MAP = 0;
COREAPI QWORD *volatile HEAPK;

void INT0E(INTERRUPT_STACK *stack)
{
	QWORD addr = __readcr2();
	if (addr >= 0xFFFF800000000000ULL)
	{
		if (addr < 0xFFFF808000000000ULL)
		{
			QWORD phyAddr = addr & 0x0000007FFFE00000ULL;
			addr &= ~((1ULL << 21) - 1);
			virtual_mapping(phyAddr, addr, 1, PAGE_2M, PAGING_WRITE);
			return;
		}
	}
	if (addr >= 0x100 && addr < 0x0000800000000000ULL)
	{
		PROCESS_CONTROL_BLOCK *pcb = current_process();
		if (!pcb)
			goto PANIC;
		LINEAR_MEMORY_BLOCK *pmm = pcb->VMMA;
		while (pmm)
		{
			if ((pmm->ADDR <= addr) && ((pmm->ADDR + pmm->SIZE) > addr) && (pmm->TYPE == VMM_TYPE_COMMIT))
			{
				virtual_mapping(alloc_physical_memory(1, 0), addr, 1, PAGE_4K, PAGING_USER | PAGING_WRITE);
				return;
			}
			pmm = pmm->NEXT;
		}
	}

	PANIC:
	panic(stack);
}
void setup_page_fault()
{
	register_interrupt(0x0E, INT0E);
}
QWORD *search_page_entry(QWORD virtAddr, int *pageType)
{
	QWORD addrMask = ~(0xFFFULL | PAGING_EXED);
	volatile VIRTUAL_ADDRESS va;
	va.address = virtAddr;
	QWORD *pml4 = (QWORD *) core_mapping(__readcr3() & addrMask);
	QWORD offset4 = va.offset4;
	if (!(pml4[offset4] & PAGING_PRESENT))
		return 0;
	QWORD *pdpt = (QWORD *) core_mapping(pml4[offset4] & addrMask);
	QWORD offset3 = va.offset3;
	QWORD pdpte = pdpt[offset3];
	if (!(pdpte & PAGING_PRESENT))
		return 0;
	if (pdpte & 0x80)
	{
		*pageType = PAGE_1G;
		return &pdpt[offset3];
	}
	QWORD *pd = (QWORD *) core_mapping(pdpte & addrMask);
	QWORD offset2 = va.offset2;
	QWORD pde = pd[offset2];
	if (!(pde & PAGING_PRESENT))
		return 0;
	if (pde & 0x80)
	{
		*pageType = PAGE_2M;
		return &pd[offset2];
	}
	QWORD *pt = (QWORD *) core_mapping(pde & addrMask);
	QWORD offset1 = va.offset1;
	if (!(pt[offset1] & PAGING_PRESENT))
		return 0;
	*pageType = PAGE_4K;
	return &pt[offset1];
}
LINEAR_MEMORY_BLOCK *vmm_alloc_node()
{
	LINEAR_MEMORY_BLOCK *val = 0;
	LINEAR_MEMORY_BLOCK *blks = BLOCK_HEAP;
	while (blks)
	{
		if (!blks[MEMBLK_NODE_PRE_PAGE].SIZE)
			goto NEXT_BLK;
		/*
		while (blks && (!blks[MEMBLK_NODE_PRE_PAGE].X))
			blks = (volatile MEMORY_BLOCK *) (blks[MEMBLK_NODE_PRE_PAGE].S);
		if (!blks)
			break;
		*/
		for (DWORD i = 0; i < MEMBLK_NODE_PRE_PAGE; i++)
		{
			if (!blks[i].VALD)
			{
				blks[MEMBLK_NODE_PRE_PAGE].SIZE--;
				val = blks + i;
				goto FOUND_OVER;
			}
		}
		blks[MEMBLK_NODE_PRE_PAGE].SIZE = 0;
		NEXT_BLK:;
		blks = (LINEAR_MEMORY_BLOCK *) (blks[MEMBLK_NODE_PRE_PAGE].ADDR);
	}
	FOUND_OVER:;
	if (!val)
	{
		*((DWORD *) 1) = 0; // Insufficient memory, panic
		return (LINEAR_MEMORY_BLOCK *) -1ULL;
	}

	if (blks && (blks[MEMBLK_NODE_PRE_PAGE].SIZE < 4) && (!blks[MEMBLK_NODE_PRE_PAGE].ADDR))
	{
		blks[MEMBLK_NODE_PRE_PAGE].ADDR = 1;
		QWORD pageAddr = alloc_physical_memory(1, 0);
		pageAddr = core_mapping(pageAddr);
		for (DWORD i = 0; i < (4096 >> 3); i++)
			((volatile QWORD *) pageAddr)[i] = 0;
		volatile LINEAR_MEMORY_BLOCK *newBlk = (volatile LINEAR_MEMORY_BLOCK *) pageAddr;
		newBlk[MEMBLK_NODE_PRE_PAGE].SIZE = MEMBLK_NODE_PRE_PAGE;
		blks[MEMBLK_NODE_PRE_PAGE].ADDR = pageAddr;
	}
	val->PREV = 0;
	val->NEXT = 0;
	val->ADDR = 0;
	val->SIZE = 0;
	val->XDAT = 0;
	val->VALD = 1;
	return val;
}
void vmm_free_node(LINEAR_MEMORY_BLOCK *blk)
{
	QWORD pageAddr = (QWORD) blk;
	pageAddr &= ~0xFFFULL;
	volatile LINEAR_MEMORY_BLOCK *blks = (volatile LINEAR_MEMORY_BLOCK *) pageAddr;
	blk->VALD = 0;
	blks[MEMBLK_NODE_PRE_PAGE].SIZE++;
}
void setup_memory()
{
	QWORD totalFree = 0;
	for (volatile EFI_MEMORY_REGION *beg = SYSTEM_TABLE->MEMORY; ~beg->A; beg++)
	{
		if (beg->A == 1 || beg->A == 3)
			totalFree += beg->L; // All free memory
		if (beg->A < 0x100000)
			continue; // Reserve low 1M memory
		if (beg->F == 1) // Free Memory
		{
			QWORD addr = beg->A;
			QWORD size = beg->L;
			if (!BLOCK_HEAP)
			{
				BLOCK_HEAP = (LINEAR_MEMORY_BLOCK *) core_mapping(addr);
				for (DWORD i = 0; i < MEMBLK_NODE_PRE_PAGE; i++)
					BLOCK_HEAP[i].VALD = 0;
				BLOCK_HEAP[MEMBLK_NODE_PRE_PAGE].SIZE = MEMBLK_NODE_PRE_PAGE;
				BLOCK_HEAP[MEMBLK_NODE_PRE_PAGE].ADDR = 0;
				addr += 0x1000;
				size -= 0x1000;
			}
			LINEAR_MEMORY_BLOCK *blk = vmm_alloc_node();
			blk->ADDR = addr;
			blk->SIZE = size;
			pmm_insert_link((LINEAR_MEMORY_BLOCK **) &MEMORY_MAP, blk, vmm_free_node);
		}
	}
}
void vmm_uncommit(QWORD addr, QWORD pageCount)
{
	for (; pageCount--; addr += 0x1000)
	{
		int pageType;
		QWORD *pageEntry = search_page_entry(addr, &pageType);
		if (pageEntry)
		{
			*pageEntry = 0;
			__invlpg((void *) addr);
		}
	}
}
void vmm_free(void *ref, QWORD addr, QWORD pageCount)
{
	LINEAR_MEMORY_BLOCK *blk = vmm_alloc_node();
	blk->ADDR = addr;
	blk->SIZE = pageCount << 12;
	pmm_insert_link(ref, blk, vmm_free_node);
}
QWORD vmm_alloc(void *root, QWORD *addr, QWORD pageCount, int align, DWORD type, DWORD virt)
{
	*addr += 0xFFF;
	*addr &= ~0xFFFULL;
	QWORD pc = pageCount;
	if (!pageCount)
		return 0;
	if (align > 63)
		goto ALLOC_FAILED;
	if (type > 4)
		goto ALLOC_FAILED;

	LINEAR_MEMORY_BLOCK **ref = root;
	while (*ref && pageCount)
	{
		LINEAR_MEMORY_BLOCK *node = *ref;
		ref = &node->NEXT;

		QWORD allocAddr = node->ADDR;
		if (allocAddr < *addr)
			allocAddr = *addr;
		allocAddr += ((1ULL << align) - 1);
		allocAddr &= ~((1ULL << align) - 1);
		if (node->ADDR + node->SIZE <= allocAddr)
			continue;

		QWORD size = ((node->ADDR + node->SIZE) - allocAddr);
		QWORD allocSize = size >> 12;

		if ((!virt) && (allocSize < pageCount))
			continue;
		if (virt)
		{
			if (type == VMM_TYPE_RESERVE && node->TYPE != VMM_TYPE_FREE)
				continue;
			if (type == VMM_TYPE_COMMIT && node->TYPE != VMM_TYPE_RESERVE)
				continue;
			if (type == VMM_TYPE_COMMITXF && node->TYPE != VMM_TYPE_FREE)
				continue;
			if ((type == VMM_TYPE_RESERVE || type == VMM_TYPE_COMMIT || type == VMM_TYPE_COMMITXF) && (allocSize < pageCount))
				continue;
		}


		if (allocSize > pageCount)
			allocSize = pageCount;
		size -= allocSize << 12;
		QWORD newAddr = (node->ADDR + node->SIZE) - size;

		if (virt)
		{
			if (node->TYPE != type)
			{
				BYTE blkType = type;
				if (blkType == VMM_TYPE_COMMITXF)
					blkType = VMM_TYPE_COMMIT;
				if (blkType == VMM_TYPE_UNCOMMIT)
					blkType = (node->TYPE) ? VMM_TYPE_RESERVE : VMM_TYPE_FREE;

				QWORD leftAddr = node->ADDR;
				QWORD leftSize = allocAddr - leftAddr;
				QWORD leftXdat = node->XDAT;
				QWORD midAddr = allocAddr;
				QWORD midSize = allocSize << 12;
				QWORD rightAddr = midAddr + midSize;
				QWORD rightSize = size;
				QWORD rightXdat = node->XDAT;
				if (node->TYPE == VMM_TYPE_COMMIT && (type == VMM_TYPE_UNCOMMIT || type == VMM_TYPE_FREE))
					vmm_uncommit(midAddr, allocSize);
				pmm_delete_link(root, node, vmm_free_node);

				if (leftSize)
				{
					LINEAR_MEMORY_BLOCK *blk = vmm_alloc_node();
					blk->ADDR = leftAddr;
					blk->SIZE = leftSize;
					blk->XDAT = leftXdat;
					pmm_insert_link(root, blk, vmm_free_node);
				}
				if (midSize)
				{
					LINEAR_MEMORY_BLOCK *blk = vmm_alloc_node();
					blk->ADDR = midAddr;
					blk->SIZE = midSize;
					blk->XDAT = leftXdat;
					blk->TYPE = blkType;
					pmm_insert_link(root, blk, vmm_free_node);
				}
				if (rightSize)
				{
					LINEAR_MEMORY_BLOCK *blk = vmm_alloc_node();
					blk->ADDR = rightAddr;
					blk->SIZE = rightSize;
					blk->XDAT = rightXdat;
					pmm_insert_link(root, blk, vmm_free_node);
				}
			}
		}
		else
		{
			if (allocAddr != node->ADDR)
			{
				node->SIZE = allocAddr - node->ADDR;

				if (size)
				{
					LINEAR_MEMORY_BLOCK *blk = vmm_alloc_node();
					blk->ADDR = newAddr;
					blk->SIZE = size;
					pmm_insert_link(root, blk, vmm_free_node);
				}
			}
			else if (size)
			{
				node->ADDR = newAddr;
				node->SIZE = size;
			}
			else
			{
				pmm_delete_link(root, node, vmm_free_node);
			}
		}
		if (*addr != allocAddr)
			*addr = allocAddr;
		align = 0;
		pageCount -= allocSize;
		*addr += allocSize;
	}
	if (pageCount)
		goto ALLOC_FAILED;
	*addr -= pc;
	return 0;

	ALLOC_FAILED:
	*addr = 0;
	return 1;
}
QWORD alloc_physical_memory(QWORD pageCount, int align)
{
	QWORD addr = 0;
	vmm_alloc((LINEAR_MEMORY_BLOCK **) &MEMORY_MAP, &addr, pageCount, align, VMM_TYPE_RESERVE, 0);
	return addr;
}
void __stdcall free_physical_memory(QWORD addr, QWORD pageCount)
{
	vmm_free((LINEAR_MEMORY_BLOCK **) &MEMORY_MAP, addr, pageCount);
}
void modify_page_attr(QWORD virtAddr, QWORD *pageEntry, QWORD attr)
{
	QWORD paMask = PAGING_WRITE | PAGING_USER | PAGING_EXED;
	attr &= paMask;
	*pageEntry &= ~paMask;
	*pageEntry |= attr;
	__invlpg((void *) (virtAddr));
}
void paging_attribute(QWORD virtAddr, QWORD attr)
{
	int pType = 0;
	QWORD *pEntry = search_page_entry(virtAddr, &pType);
	modify_page_attr(virtAddr, pEntry, attr);
}
void virtual_mapping(QWORD phyAddr, const QWORD virtualAddr, QWORD pageCount, int pageType, QWORD attr)
{
	QWORD addrMask = ~(0xFFFULL | PAGING_EXED);
	QWORD page = 0x1000;
	page <<= (pageType * 9);

	phyAddr &= ~(page - 1);

	volatile VIRTUAL_ADDRESS va;
	va.address = virtualAddr;
	while (pageCount--)
	{
		QWORD *pEntry = 0;
		QWORD *pml4 = (QWORD *) core_mapping(__readcr3() & addrMask);
		QWORD offset4 = va.offset4;
		// create 512G page entry
		if (!(pml4[offset4] & 1))
		{
			QWORD pageAddr = alloc_physical_memory(1, 0);
			pml4[offset4] = pageAddr | PAGING_PRESENT | PAGING_WRITE | PAGING_USER;
			QWORD *pageBuf = (QWORD *) core_mapping(pageAddr);
			pageBuf[0] = pageBuf[1] = 0;
			__memset128(pageBuf, pageBuf, 512);
		}

		// create 1G page
		QWORD *pdpt = (QWORD *) core_mapping(pml4[offset4] & addrMask);
		QWORD offset3 = va.offset3;
		if (pageType == PAGE_1G)
		{
			pdpt[offset3] = phyAddr | PAGING_PRESENT | PAGING_PAGESIZE;
			pEntry = pdpt + offset3;
			goto CONTINU;
		}
		// create 1G page entry
		if (!(pdpt[offset3] & 1))
		{
			QWORD pageAddr = alloc_physical_memory(1, 0);
			pdpt[offset3] = pageAddr | PAGING_PRESENT | PAGING_WRITE | PAGING_USER;
			QWORD *pageBuf = (QWORD *) core_mapping(pageAddr);
			pageBuf[0] = pageBuf[1] = 0;
			__memset128(pageBuf, pageBuf, 512);
		}

		// create 2M page
		QWORD *pd = (QWORD *) core_mapping(pdpt[offset3] & addrMask);
		QWORD offset2 = va.offset2;
		if (pageType == PAGE_2M)
		{
			pd[offset2] = phyAddr | PAGING_PRESENT | PAGING_PAGESIZE;
			pEntry = pd + offset2;
			goto CONTINU;
		}
		// create 2M page entry
		if (!(pd[offset2] & 1))
		{
			QWORD pageAddr = alloc_physical_memory(1, 0);
			pd[offset2] = pageAddr | PAGING_PRESENT | PAGING_WRITE | PAGING_USER;
			QWORD *pageBuf = (QWORD *) core_mapping(pageAddr);
			pageBuf[0] = pageBuf[1] = 0;
			__memset128(pageBuf, pageBuf, 512);
		}

		// create 4K page
		QWORD *pt = (QWORD *) core_mapping(pd[offset2] & addrMask);
		QWORD offset1 = va.offset1;
		pt[offset1] = phyAddr | PAGING_PRESENT;
		pEntry = pt + offset1;


		CONTINU:;
		modify_page_attr(va.address, pEntry, attr);
		phyAddr += page;
		va.address += page;
	}
}
QWORD physical_address(QWORD virtAddr)
{
	int pType = 0;
	QWORD *pEntry = search_page_entry(virtAddr, &pType);
	if (pType == PAGE_1G)
		return (*pEntry & (~(PAGE_1G_OFFSET | PAGING_EXED))) | (virtAddr & PAGE_1G_OFFSET);
	if (pType == PAGE_2M)
		return (*pEntry & (~(PAGE_2M_OFFSET | PAGING_EXED))) | (virtAddr & PAGE_2M_OFFSET);
	return (*pEntry & (~(PAGE_4K_OFFSET | PAGING_EXED))) | (virtAddr & PAGE_4K_OFFSET);
}
void *heap_alloc(QWORD allocSize)
{
	if (!HEAPK)
	{
		// Create a new Heap with 1*4K page
		QWORD phyAddr = alloc_physical_memory(1, 0);
		// Mapping to Heap Space
		QWORD heapBase = 0xFFFF808000000000ULL;
		virtual_mapping(phyAddr, heapBase, 1, PAGE_4K, PAGING_WRITE);
		HEAPK = (QWORD *) heapBase;
		// Initial block size, free, lastblock
		HEAPK[0] = 0xFF8 | HEAP_FLAG_LAST;
	}
	// At least 8 byte
	if (!allocSize)
		allocSize = 8;
	// Align at 8 byte boundary
	allocSize += 7;
	allocSize &= ~(7ULL);
	// Foreach heap blocks
	QWORD *heap = HEAPK;
	while (1)
	{
		while (1)
		{
			// Free and sufficient block
			if (!(*heap & HEAP_FLAG_USING) && (*heap >= allocSize))
			{
				// Get block flags
				QWORD blockFlag = *heap & 7;
				// Get block size
				QWORD blockSize = *heap & (~7ULL);
				// Set USING flag
				*heap |= HEAP_FLAG_USING;
				// Alloc addr is next QWORD
				void *allocAddr = heap + 1;
				// If block size greater than the required size, split the block
				if (blockSize > allocSize)
				{
					// Split to 2 blocks
					// Clear last flag and update block size
					*heap = allocSize | HEAP_FLAG_USING;
					// Goto next block head
					heap += 1 + (allocSize >> 3);
					// Set block flags and size
					*heap = blockSize - allocSize - 8;
					*heap |= blockFlag;
				}
				return allocAddr;
			}
			// Last block
			if (*heap & HEAP_FLAG_LAST)
				break;
			// Next block
			heap += 1 + (*heap >> 3);
		}
		// No free block
		// Expand the heap
		while ((*heap & (~7ULL)) < allocSize)
		{
			QWORD *heapEnd = heap + 1 + (*heap >> 3);
			QWORD heapEndAddr = (QWORD) heapEnd;
			// Allocate one 4K page
			QWORD phyPage = alloc_physical_memory(1, 0);
			if (!phyPage)
			{
				// No free page, kernel panic
				*((QWORD *) -1ULL) = 1;
			}
			// Mapping page to heap end
			virtual_mapping(phyPage, heapEndAddr, 1, PAGE_4K, PAGING_WRITE);
			// Update block size
			*heap += 0x1000;
		}
	}
}
void heap_free(const volatile void *addr)
{
	// No heap created
	if (!HEAPK)
		return;

	QWORD *heap = HEAPK;
	QWORD *prev = 0;
	for (; ; prev = heap, heap += 1 + (*heap >> 3))
	{
		// Not this block
		if ((heap + 1) != addr)
		{
			if (*heap & HEAP_FLAG_LAST)
				break;
			continue;
		}

		// Found the block
		// Free the block by clear using flag
		*heap &= ~HEAP_FLAG_USING;
		// Try merge with prev free block
		if (prev && !(*prev & HEAP_FLAG_USING))
		{
			*prev += 8 + (*heap & ~7ULL);
			heap = prev;
		}

		if ((*heap & HEAP_FLAG_LAST))
			return;
		// Try merge with next block
		QWORD *next = heap + 1 + (*heap >> 3);
		if (!(*next & HEAP_FLAG_USING))
		{
			*heap += 8 + (*next & ~7ULL);
			*heap |= (*next & 7);
		}
		return;
	}
}
QWORD virtual_alloc(QWORD proc, QWORD *virtAddr, QWORD allocSize, DWORD allocType)
{
	if (*virtAddr + (allocSize << 12) > 0x0000800000000000ULL)
	{
		*virtAddr = 0;
		return -1;
	}

	if (__getcs() & 3)
	{
		SYSCALL_VIRTUAL_ALLOC call;
		call.TYPE = SYSCALL_TYPE_VIRTUAL_ALLOC;
		call.PROC = proc;
		call.ADDR = virtAddr;
		call.SIZE = allocSize;
		call.ATYP = allocType;
		__syscall(&call);
	}
	if (!proc)
		return 0xFFF;

	if (proc == CURRENT_PROCESS_HANDLE)
		proc = (QWORD) current_process();
	PROCESS_CONTROL_BLOCK *currproc = (PROCESS_CONTROL_BLOCK *) core_mapping(proc);
	return vmm_alloc(&currproc->VMMA, virtAddr, allocSize, 0, allocType, 1);
}