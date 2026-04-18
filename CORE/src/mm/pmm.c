#include <mm/pmm.h>
#include <core.h>

int pmm_merge(LINEAR_MEMORY_BLOCK *dst, LINEAR_MEMORY_BLOCK *src)
{
	if (!dst->VALD || !src->VALD)
		return 0;
	if (dst->XDAT != src->XDAT)
		return 0;
	if ((src->ADDR >= dst->ADDR) && (src->ADDR <= (dst->ADDR + dst->SIZE)))
	{
		QWORD z = src->ADDR + src->SIZE;
		if (dst->ADDR + dst->SIZE > z)
			z = dst->ADDR + dst->SIZE;
		dst->SIZE = z - dst->ADDR;
		return 1;
	}
	return 0;
}
void pmm_delete_link(LINEAR_MEMORY_BLOCK **root, LINEAR_MEMORY_BLOCK *blk, void (*freeNode)(LINEAR_MEMORY_BLOCK *))
{
	LINEAR_MEMORY_BLOCK *lnode = blk->PREV;
	LINEAR_MEMORY_BLOCK *rnode = blk->NEXT;
	LINEAR_MEMORY_BLOCK **lref = root;
	if (lnode)
		lref = &lnode->NEXT;
	*lref = blk->NEXT;
	if (rnode)
		rnode->PREV = blk->PREV;
	freeNode(blk);
}
void pmm_insert_link(LINEAR_MEMORY_BLOCK **root, LINEAR_MEMORY_BLOCK *blk, void (*freeNode)(LINEAR_MEMORY_BLOCK *))
{
	LINEAR_MEMORY_BLOCK **lref = root;
	LINEAR_MEMORY_BLOCK *prev = 0;
	while (*lref)
	{
		LINEAR_MEMORY_BLOCK *node = (*lref);
		if (pmm_merge(node, blk))
		{
			freeNode(blk);
			LINEAR_MEMORY_BLOCK *next = (node->NEXT);
			if (next && pmm_merge(node, next))
				pmm_delete_link(root, next, freeNode);
			return;
		}
		if (pmm_merge(blk, node))
		{
			node->ADDR = blk->ADDR;
			node->SIZE = blk->SIZE;
			freeNode(blk);
			return;
		}
		if (node->ADDR >= blk->ADDR)
		{
			*lref = (blk);
			blk->PREV = (prev);
			blk->NEXT = (node);
			node->PREV = (blk);
			return;
		}
		prev = node;
		lref = &node->NEXT;
	}
	*lref = (blk);
	blk->PREV = (prev);
}