#include <memory/virtmem.h>
#include <memory/memblk.h>
#include <core.h>
#include <intrinsic.h>

#define MEMBLK_NODE_PRE_PAGE 0x7F

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

		QWORD addr = node->A;
		addr += ((1ULL << align) - 1);
		addr &= ~((1ULL << align) - 1);
		QWORD size = ((node->A + node->S) - addr);
		QWORD allocSize = size >> 12;
		QWORD newAddr;

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
		newAddr = (node->A + node->S) - size;

		if (addr != node->A)
		{
			node->S = addr - node->A;

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

		return newAddr;
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