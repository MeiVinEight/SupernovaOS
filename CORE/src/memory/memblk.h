//
// Created by MeiVi on 2026/03/19.
//

#ifndef SUPERNOVA_MEMBLK_H
#define SUPERNOVA_MEMBLK_H

#include <types.h>

#define MEMBLK_NODE_PRE_PAGE 0x7F

#define MEMBLK_LROTATE 0
#define MEMBLK_RROTATE 1

#define MEMBLK_BIG_LES 0
#define MEMBLK_SML_GRT 1

typedef struct _MEMORY_BLOCK
{
	DWORD P; // Parent Node
	DWORD L; // Left Node
	DWORD R; // Right Node
	WORD  H; // Subtree height
	WORD  X; // Memory block flags
	QWORD A; // Memory block address
	QWORD S; // Memory block size
} MEMORY_BLOCK;

volatile MEMORY_BLOCK *memblk_alloc_node();
void memblk_free_node(volatile MEMORY_BLOCK *blk);
volatile MEMORY_BLOCK *memblk_mapping_node(DWORD addr);
DWORD memblk_node_addr(volatile MEMORY_BLOCK *node);
volatile MEMORY_BLOCK *memblk_pval(DWORD addr);
volatile MEMORY_BLOCK *memblk_lval(DWORD addr);
volatile MEMORY_BLOCK *memblk_rval(DWORD addr);
WORD memblk_hval(DWORD addr);
WORD memblk_xval(DWORD addr);
QWORD memblk_aval(DWORD addr);
QWORD memblk_sval(DWORD addr);
volatile DWORD *memblk_pref(DWORD addr);
volatile DWORD *memblk_lref(DWORD addr);
volatile DWORD *memblk_rref(DWORD addr);
volatile WORD  *memblk_href(DWORD addr);
volatile WORD  *memblk_xref(DWORD addr);
volatile QWORD *memblk_aref(DWORD addr);
volatile QWORD *memblk_sref(DWORD addr);
volatile DWORD *memblk_node_reference(volatile DWORD *rootref, volatile MEMORY_BLOCK *blk);
WORD memblk_tree_height(volatile MEMORY_BLOCK *blk);
void memblk_adjust_height(volatile MEMORY_BLOCK *blk);


#endif //SUPERNOVA_MEMBLK_H
