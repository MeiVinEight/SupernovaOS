#include <memory/virtmem.h>
#include <memory/memblk.h>
#include <core.h>
#include <intrinsic.h>

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

COREAPI volatile MEMORY_BLOCK *volatile BLOCK_HEAP = 0;
COREAPI volatile DWORD MEMORY_MAP = 0;
COREAPI QWORD *HEAPK;

volatile MEMORY_BLOCK *alloc_memblk()
{
	volatile MEMORY_BLOCK *volatile val = 0;
	volatile MEMORY_BLOCK *volatile blks = BLOCK_HEAP;
	while (blks)
	{
		if (!blks[MEMBLK_NODE_PRE_PAGE].X)
			goto NEXT_BLK;
		/*
		while (blks && (!blks[MEMBLK_NODE_PRE_PAGE].X))
			blks = (volatile MEMORY_BLOCK *) (blks[MEMBLK_NODE_PRE_PAGE].S);
		if (!blks)
			break;
		*/
		for (DWORD i = 0; i < MEMBLK_NODE_PRE_PAGE; i++)
		{
			if (!(blks[i].X & 1))
			{
				blks[i].X |= 1;
				blks[MEMBLK_NODE_PRE_PAGE].X--;
				val = blks + i;
				goto FOUND_OVER;
			}
		}
		blks[MEMBLK_NODE_PRE_PAGE].X = 0;
		NEXT_BLK:;
		blks = (volatile MEMORY_BLOCK *) (blks[MEMBLK_NODE_PRE_PAGE].S);
	}
	FOUND_OVER:;
	if (blks && (blks[MEMBLK_NODE_PRE_PAGE].X < 4) && (!blks[MEMBLK_NODE_PRE_PAGE].S))
	{
		QWORD pageCount = 1;
		QWORD pageAddr = alloc_physical_memory(&pageCount, 0, 0);
		pageAddr = core_mapping(pageAddr);
		for (DWORD i = 0; i < (4096 >> 3); i++)
			((volatile QWORD *) pageAddr)[i] = 0;
		volatile MEMORY_BLOCK *newBlk = (volatile MEMORY_BLOCK *) pageAddr;
		newBlk[MEMBLK_NODE_PRE_PAGE].X = MEMBLK_NODE_PRE_PAGE;
		blks[MEMBLK_NODE_PRE_PAGE].S = pageAddr;
	}
	val->P = 0;
	val->L = 0;
	val->R = 0;
	val->H = 0;
	val->X = 1;
	val->A = 0;
	val->S = 0;
	return val;
}
void free_memblk(volatile MEMORY_BLOCK *blk)
{
	QWORD pageAddr = (QWORD) blk;
	pageAddr &= ~0xFFFULL;
	volatile MEMORY_BLOCK *blks = (volatile MEMORY_BLOCK *) pageAddr;
	blk->X = 0;
	blks[MEMBLK_NODE_PRE_PAGE].X++;
}
void setup_memory()
{
	QWORD totalFree = 0;
	for (volatile EFI_MEMORY_REGION *volatile beg = SYSTEM_TABLE->MEMORY; ~beg->A; beg++)
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
				BLOCK_HEAP = (volatile MEMORY_BLOCK *) core_mapping(addr);
				for (DWORD i = 0; i < MEMBLK_NODE_PRE_PAGE; i++)
					BLOCK_HEAP[i].X = 0;
				BLOCK_HEAP[MEMBLK_NODE_PRE_PAGE].X = MEMBLK_NODE_PRE_PAGE;
				BLOCK_HEAP[MEMBLK_NODE_PRE_PAGE].S = 0;
				addr += 0x1000;
				size -= 0x1000;
			}
			volatile MEMORY_BLOCK *blk = alloc_memblk();
			if (!blk)
				break;
			blk->A = addr;
			blk->S = size;
			memblk_insert_link(&MEMORY_MAP, blk, free_memblk);
		}
	}
}
QWORD __stdcall alloc_physical_memory(QWORD *pageCount, int align, int continu)
{
	volatile DWORD *volatile ref = &MEMORY_MAP;
	while (*ref)
	{
		volatile MEMORY_BLOCK *node = memblk_mapping_node(*ref);

		QWORD allocAddr = node->A;
		allocAddr += ((1ULL << align) - 1);
		allocAddr &= ~((1ULL << align) - 1);
		QWORD size = ((node->A + node->S) - allocAddr);
		QWORD allocSize = size >> 12;

		if (continu)
		{
			if (allocSize < *pageCount)
			{
				ref = &node->R;
				continue;
			}
		}
		else
		{
			if (*pageCount > allocSize)
				*pageCount = allocSize;
		}
		size -= *pageCount << 12;
		QWORD newAddr = (node->A + node->S) - size;

		if (allocAddr != node->A)
		{
			node->S = allocAddr - node->A;

			if (size)
			{
				volatile MEMORY_BLOCK *blk = alloc_memblk();
				if (!blk)
				{
					node->S = newAddr + size;
					return 0;
				}
				blk->A = newAddr;
				blk->S = size;
				memblk_insert_link(&MEMORY_MAP, blk, free_memblk);
			}
		}
		else if (size)
		{
			node->A = newAddr;
			node->S = size;
		}
		else
		{
			memblk_delete_link(&MEMORY_MAP, node, free_memblk);
		}
		return allocAddr;
	}
	return 0;
}
void __stdcall free_physical_memory(QWORD addr, QWORD pageCount)
{
	volatile MEMORY_BLOCK *blk = alloc_memblk();
	blk->A = addr;
	blk->S = pageCount << 12;
	memblk_insert_link(&MEMORY_MAP, blk, free_memblk);
}
void virtual_mapping(QWORD phyAddr, const QWORD virtualAddr, QWORD pageCount, int pageType)
{
	QWORD addrMask = ~0xFFFULL;
	QWORD page = 0x1000;
	page <<= (pageType * 9);

	phyAddr &= ~(page - 1);

	volatile VIRTUAL_ADDRESS va;
	va.address = virtualAddr;
	while (pageCount--)
	{
		QWORD *pml4 = (QWORD *) core_mapping(__readcr3() & addrMask);
		QWORD offset4 = va.offset4;
		if (!(pml4[offset4] & 1))
		{
			QWORD allpc = 1;
			QWORD pageAddr = alloc_physical_memory(&allpc, 0, 0);
			pml4[offset4] = pageAddr | 3;
			QWORD *pageBuf = (QWORD *) core_mapping(pageAddr);
			pageBuf[0] = pageBuf[1] = 0;
			__memset128(pageBuf, pageBuf, 512);
		}

		QWORD *pdpt = (QWORD *) core_mapping(pml4[offset4] & addrMask);
		QWORD offset3 = va.offset3;
		if (pageType == PAGE_1G)
		{
			pdpt[offset3] = phyAddr | 0x83;
			goto CONTINU;
		}
		if (!(pdpt[offset3] & 1))
		{
			QWORD allpc = 1;
			QWORD pageAddr = alloc_physical_memory(&allpc, 0, 0);
			pdpt[offset3] = pageAddr | 3;
			QWORD *pageBuf = (QWORD *) core_mapping(pageAddr);
			pageBuf[0] = pageBuf[1] = 0;
			__memset128(pageBuf, pageBuf, 512);
		}

		QWORD *pd = (QWORD *) core_mapping(pdpt[offset3] & addrMask);
		QWORD offset2 = va.offset2;
		if (pageType == PAGE_2M)
		{
			pd[offset2] = phyAddr | 0x83;
			goto CONTINU;
		}
		if (!(pd[offset2] & 1))
		{
			QWORD allpc = 1;
			QWORD pageAddr = alloc_physical_memory(&allpc, 0, 0);
			pd[offset2] = pageAddr | 3;
			QWORD *pageBuf = (QWORD *) core_mapping(pageAddr);
			pageBuf[0] = pageBuf[1] = 0;
			__memset128(pageBuf, pageBuf, 512);
		}

		QWORD *pt = (QWORD *) core_mapping(pd[offset2] & addrMask);
		QWORD offset1 = va.offset1;
		pt[offset1] = phyAddr | 0x03;


		CONTINU:;
		phyAddr += page;
		va.address += page;
	}
}
QWORD physical_address(QWORD virtAddr)
{
	QWORD addrMask = ~0xFFFULL;
	volatile VIRTUAL_ADDRESS va;
	va.address = virtAddr;
	QWORD *pml4 = (QWORD *) core_mapping(__readcr3() & addrMask);
	QWORD offset4 = va.offset4;
	QWORD *pdpt = (QWORD *) core_mapping(pml4[offset4] & addrMask);
	QWORD offset3 = va.offset3;
	QWORD pdpte = pdpt[offset3];
	if (pdpte & 0x80)
		return (pdpte & ~(0x3FFFFFFFULL)) | (virtAddr & 0x3FFFFFFF);
	QWORD *pd = (QWORD *) core_mapping(pdpte & addrMask);
	QWORD offset2 = va.offset2;
	QWORD pde = pd[offset2];
	if (pde & 0x80)
		return (pde & ~(0x001FFFFFULL)) | (virtAddr & 0x001FFFFF);
	QWORD *pt = (QWORD *) core_mapping(pde & addrMask);
	QWORD offset1 = va.offset1;
	QWORD pte = pt[offset1];
	return (pte & ~(0x00000FFFULL)) | va.offset0;
}
void *heap_alloc(QWORD allocSize)
{
	if (!HEAPK)
	{
		// Create a new Heap with 1*4K page
		QWORD pc = 1;
		QWORD phyAddr = alloc_physical_memory(&pc, 0, 0);
		// Mapping to Heap Space
		QWORD heapBase = 0xFFFF810000000000ULL;
		virtual_mapping(phyAddr, heapBase, 1, PAGE_4K);
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
			QWORD pc = 1;
			QWORD phyPage = alloc_physical_memory(&pc, 0, 0);
			if (!phyPage)
			{
				// No free page, kernel panic
				*((QWORD *) -1ULL) = 1;
			}
			// Mapping page to heap end
			virtual_mapping(phyPage, heapEndAddr, 1, PAGE_4K);
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