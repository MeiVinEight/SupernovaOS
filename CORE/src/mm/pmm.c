#include <mm/pmm.h>
#include <core.h>

volatile LINEAR_MEMORY_BLOCK *memblk_mapping_node(DWORD addr)
{
	if (addr)
		return (volatile LINEAR_MEMORY_BLOCK *) core_mapping(addr);
	return 0;
}
DWORD memblk_node_addr(volatile LINEAR_MEMORY_BLOCK *node)
{
	return ((QWORD) node) & 0xFFFFFFFF;
}
int memblk_merge(volatile LINEAR_MEMORY_BLOCK *dst, volatile LINEAR_MEMORY_BLOCK *src)
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
void memblk_delete_link(volatile DWORD *root, volatile LINEAR_MEMORY_BLOCK *blk, void (*freeNode)(volatile LINEAR_MEMORY_BLOCK *))
{
	volatile LINEAR_MEMORY_BLOCK *lnode = memblk_mapping_node(blk->L);
	volatile LINEAR_MEMORY_BLOCK *rnode = memblk_mapping_node(blk->R);
	volatile DWORD *lref = root;
	if (lnode)
		lref = &lnode->R;
	*lref = blk->R;
	if (rnode)
		rnode->L = blk->L;
	freeNode(blk);
}
void memblk_insert_link(volatile DWORD *root, volatile LINEAR_MEMORY_BLOCK *blk, void (*freeNode)(volatile LINEAR_MEMORY_BLOCK *))
{
	volatile DWORD *volatile lref = root;
	volatile LINEAR_MEMORY_BLOCK *volatile prev = 0;
	while (*lref)
	{
		volatile LINEAR_MEMORY_BLOCK *node = memblk_mapping_node(*lref);
		if (memblk_merge(node, blk))
		{
			freeNode(blk);
			volatile LINEAR_MEMORY_BLOCK *next = memblk_mapping_node(node->R);
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