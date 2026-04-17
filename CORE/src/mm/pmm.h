//
// Created by MeiVi on 2026/03/19.
//

#ifndef SUPERNOVA_MEMBLK_H
#define SUPERNOVA_MEMBLK_H

#include <types.h>

#define MEMBLK_LROTATE 0
#define MEMBLK_RROTATE 1

#define MEMBLK_BIG_LES 0
#define MEMBLK_SML_GRT 1

typedef struct _LINEAR_MEMORY_BLOCK
{
	DWORD L; // Left Node
	DWORD R; // Right Node
	WORD  X; // Memory block flags
	QWORD A; // Memory block address
	QWORD S; // Memory block size
} LINEAR_MEMORY_BLOCK;

volatile LINEAR_MEMORY_BLOCK *memblk_mapping_node(DWORD addr);
DWORD memblk_node_addr(volatile LINEAR_MEMORY_BLOCK *node);
void memblk_delete_link(volatile DWORD *root, volatile LINEAR_MEMORY_BLOCK *blk, void (*freeNode)(volatile LINEAR_MEMORY_BLOCK *));
void memblk_insert_link(volatile DWORD *root, volatile LINEAR_MEMORY_BLOCK *blk, void (*freeNode)(volatile LINEAR_MEMORY_BLOCK *));


#endif //SUPERNOVA_MEMBLK_H
