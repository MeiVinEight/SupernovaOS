#include <mm/pmm.h>
#include <core.h>

volatile MEMORY_BLOCK *memblk_mapping_node(DWORD addr)
{
	if (addr)
		return (volatile MEMORY_BLOCK *) core_mapping(addr);
	return 0;
}
DWORD memblk_node_addr(volatile MEMORY_BLOCK *node)
{
	return ((QWORD) node) & 0xFFFFFFFF;
}
volatile MEMORY_BLOCK *memblk_pval(DWORD addr)
{
	if (addr)
		return memblk_mapping_node(memblk_mapping_node(addr)->P);
	return 0;
}
volatile MEMORY_BLOCK *memblk_lval(DWORD addr)
{
	if (addr)
		return memblk_mapping_node(memblk_mapping_node(addr)->L);
	return 0;
}
volatile MEMORY_BLOCK *memblk_rval(DWORD addr)
{
	if (addr)
		return memblk_mapping_node(memblk_mapping_node(addr)->R);
	return 0;
}
WORD memblk_hval(DWORD addr)
{
	if (addr)
		return memblk_mapping_node(addr)->H;
	return 0;
}
WORD memblk_xval(DWORD addr)
{
	if (addr)
		return memblk_mapping_node(addr)->X;
	return 0;
}
QWORD memblk_aval(DWORD addr)
{
	if (addr)
		return memblk_mapping_node(addr)->A;
	return 0;
}
QWORD memblk_sval(DWORD addr)
{
	if (addr)
		return memblk_mapping_node(addr)->S;
	return 0;
}
volatile DWORD *memblk_pref(DWORD addr)
{
	if (addr)
		return &memblk_mapping_node(addr)->P;
	return 0;
}
volatile DWORD *memblk_lref(DWORD addr)
{
	if (addr)
		return &memblk_mapping_node(addr)->L;
	return 0;
}
volatile DWORD *memblk_rref(DWORD addr)
{
	if (addr)
		return &memblk_mapping_node(addr)->R;
	return 0;
}
int memblk_merge(volatile MEMORY_BLOCK *dst, volatile MEMORY_BLOCK *src)
{
	if ((src->A >= dst->A) && (src->A <= (dst->A + dst->S)))
	{
		QWORD z = src->A + src->S;
		if (dst->A + dst->S > z)
			z = dst->A + dst->S;
		dst->S = z - dst->A;
		return 1;
	}
	return 0;
}

volatile DWORD *memblk_node_reference(volatile DWORD *rootref, volatile MEMORY_BLOCK *blk)
{
	if (blk->P)
	{
		if (memblk_lval(blk->P) == blk)
			return memblk_lref(blk->P);
		return memblk_rref(blk->P);
	}
	return rootref;
}
WORD memblk_tree_height(volatile MEMORY_BLOCK *blk)
{
	if (blk)
		return blk->H;
	return 0;
}
void memblk_adjust_height(volatile MEMORY_BLOCK *blk)
{
	WORD lh = memblk_tree_height(memblk_mapping_node(blk->L));
	WORD rh = memblk_tree_height(memblk_mapping_node(blk->R));
	blk->H = ((lh > rh) ? lh : rh) + 1;
}
void memblk_rotate(volatile DWORD *ref, DWORD type)
{
	volatile MEMORY_BLOCK *block = memblk_mapping_node(*ref);
	volatile MEMORY_BLOCK *pnode = memblk_mapping_node(block->P);
	volatile MEMORY_BLOCK *lnode = memblk_mapping_node(block->L);
	volatile MEMORY_BLOCK *rnode = memblk_mapping_node(block->R);
	switch (type)
	{
		case MEMBLK_LROTATE:
			if (rnode->L)
				*memblk_pref(rnode->L) = memblk_node_addr(block);
			block->R = rnode->L;
			block->P = memblk_node_addr(rnode);
			rnode->L = memblk_node_addr(block);
			rnode->P = memblk_node_addr(pnode);
			*ref = memblk_node_addr(rnode);
			break;
		case MEMBLK_RROTATE:
			if (lnode->R)
				*memblk_pref(lnode->R) = memblk_node_addr(block);
			block->L = lnode->R;
			block->P = memblk_node_addr(lnode);
			lnode->R = memblk_node_addr(block);
			lnode->P = memblk_node_addr(pnode);
			*ref = memblk_node_addr(lnode);
			break;
		default: return;
	}
	memblk_adjust_height(block);
	memblk_adjust_height(memblk_mapping_node(*ref));
}
void memblk_adjust_tree(volatile DWORD *root, volatile MEMORY_BLOCK *volatile blk)
{
	while (blk)
	{
		volatile DWORD *ref = memblk_node_reference(root, blk);
		volatile MEMORY_BLOCK *lnode = memblk_mapping_node(blk->L);
		volatile MEMORY_BLOCK *rnode = memblk_mapping_node(blk->R);

		QWORD lh = memblk_tree_height(lnode);
		QWORD rh = memblk_tree_height(rnode);
		if (lh > (rh + 1))
		{
			// Right rotate
			if (memblk_tree_height(memblk_mapping_node(lnode->R)) > memblk_tree_height(memblk_mapping_node(lnode->L)))
				memblk_rotate(&blk->L, MEMBLK_LROTATE);
			memblk_rotate(ref, MEMBLK_RROTATE);
		}
		else if (rh > (lh + 1))
		{
			// Left rotate
			if (memblk_tree_height(memblk_mapping_node(rnode->L)) > memblk_tree_height(memblk_mapping_node(rnode->R)))
				memblk_rotate(&blk->R, MEMBLK_RROTATE);
			memblk_rotate(ref, MEMBLK_LROTATE);
		}
		memblk_adjust_height(memblk_mapping_node(*ref));
		blk = memblk_pval(*ref);
	}
}
volatile MEMORY_BLOCK *memblk_search_node(volatile DWORD *root, QWORD address, DWORD type)
{
	volatile MEMORY_BLOCK *volatile next = 0;
	volatile MEMORY_BLOCK *volatile curr = memblk_mapping_node(*root);
	switch (type)
	{
		case MEMBLK_BIG_LES:
			while (curr)
			{
				if (curr->A <= address)
				{
					next = curr;
					curr = memblk_mapping_node(curr->R);
				}
				else
					curr = memblk_mapping_node(curr->L);
			}
			return next;
		case MEMBLK_SML_GRT:
			while (curr)
			{
				if (curr->A >= address)
				{
					next = curr;
					curr = memblk_mapping_node(curr->L);
				}
				else
					curr = memblk_mapping_node(curr->R);
			}
			return next;
		default: return 0;
	}
}
void memblk_delete_node(volatile DWORD *root, volatile MEMORY_BLOCK *blk, void (*freeNode)(volatile MEMORY_BLOCK *))
{
	if (blk->L && blk->R)
	{
		// Left and right children are not null
		// Find greatest node which is less than this block.
		volatile MEMORY_BLOCK *volatile prev = memblk_mapping_node(blk->L);
		while (prev->R)
			prev = memblk_mapping_node(prev->R);
		// Copy data from prev to block, means block deleted and prev is invalid.
		blk->H = prev->H;
		blk->X = prev->X;
		blk->A = prev->A;
		blk->S = prev->S;
		// Delete node prev
		// Node's right children is null
		// Only needs to move left children to parent's pointer
		*memblk_node_reference(root, prev) = prev->L;
		if (prev->L)
			*memblk_pref(prev->L) = prev->P;
		// Make block point to the deleted node
		blk = prev;
	}
	else
	{
		// Has up to one child
		volatile MEMORY_BLOCK *child = memblk_mapping_node(blk->L | blk->R);
		// Move child to parent's pointer
		*memblk_node_reference(root, blk) = memblk_node_addr(child);
		if (child)
			child->P = blk->P;
	}
	// Recursively adjust tree
	memblk_adjust_tree(root, memblk_mapping_node(blk->P));
	// Free node memory
	freeNode(blk);
}
void memblk_insert_node(volatile DWORD *root, volatile MEMORY_BLOCK *blk, void (*freeNode)(volatile MEMORY_BLOCK *))
{
	// Merge
	// Find smallest value which greater than this block
	volatile MEMORY_BLOCK *next = memblk_search_node(root, blk->A, MEMBLK_SML_GRT);
	// Find biggest value which less than this block
	volatile MEMORY_BLOCK *prev = memblk_search_node(root, blk->A, MEMBLK_BIG_LES);

	// Check merge
	if (prev && ((prev->A + prev->S) >= blk->A))
	{
		QWORD A = prev->A;
		QWORD Z = blk->A + blk->S;
		if (prev->A + prev->S > Z)
			Z = prev->A + prev->S;
		blk->A = A;
		blk->S = Z - A;
		memblk_delete_node(root, prev, freeNode);
	}
	if (next && ((blk->A + blk->S) >= next->A))
	{
		QWORD A = blk->A;
		QWORD Z = next->A + next->S;
		if (blk->A + blk->S > Z)
			Z = blk->A + blk->S;
		blk->A = A;
		blk->S = Z - A;
		memblk_delete_node(root, next, freeNode);
	}

	// Insert block into memory map
	volatile MEMORY_BLOCK *volatile parn = 0;
	volatile DWORD *volatile ref = root;
	while (*ref)
	{
		if (blk->A > memblk_mapping_node(*ref)->A)
		{
			// Insert into right subtree
			parn = memblk_mapping_node(*ref);
			ref = &parn->R;
		}
		else
		{
			// Insert into left subtree
			parn = memblk_mapping_node(*ref);
			ref = &parn->L;
		}
	}
	blk->P = memblk_node_addr(parn);
	*ref = memblk_node_addr(blk);
	memblk_adjust_tree(root, blk);
}

void memblk_delete_link(volatile DWORD *root, volatile MEMORY_BLOCK *blk, void (*freeNode)(volatile MEMORY_BLOCK *))
{
	volatile MEMORY_BLOCK *lnode = memblk_mapping_node(blk->L);
	volatile MEMORY_BLOCK *rnode = memblk_mapping_node(blk->R);
	volatile DWORD *lref = root;
	if (lnode)
		lref = &lnode->R;
	*lref = blk->R;
	if (rnode)
		rnode->L = blk->L;
	freeNode(blk);
}
void memblk_insert_link(volatile DWORD *root, volatile MEMORY_BLOCK *blk, void (*freeNode)(volatile MEMORY_BLOCK *))
{
	volatile DWORD *volatile lref = root;
	volatile MEMORY_BLOCK *volatile prev = 0;
	while (*lref)
	{
		volatile MEMORY_BLOCK *node = memblk_mapping_node(*lref);
		if (memblk_merge(node, blk))
		{
			freeNode(blk);
			volatile MEMORY_BLOCK *next = memblk_mapping_node(node->R);
			if (next && memblk_merge(node, next))
				memblk_delete_link(root, blk, freeNode);
			return;
		}
		if (memblk_merge(blk, node))
		{
			node->A = blk->A;
			node->S = blk->S;
			freeNode(blk);
			return;
		}
		if (node->A >= blk->A)
		{
			*lref = memblk_node_addr(blk);
			blk->L = memblk_node_addr(prev);
			blk->R = memblk_node_addr(node);
			node->L = memblk_node_addr(blk);
			return;
		}
		prev = node;
		lref = &node->R;
	}
	*lref = memblk_node_addr(blk);
	blk->L = memblk_node_addr(prev);
}