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

typedef struct _LINEAR_MEMORY_BLOCK LINEAR_MEMORY_BLOCK;
struct _LINEAR_MEMORY_BLOCK
{
	LINEAR_MEMORY_BLOCK *PREV;    // Left Node
	LINEAR_MEMORY_BLOCK *NEXT;    // Right Node
	QWORD                ADDR;    // Memory block address
	union
	{
		struct
		{
			QWORD        SIZE:56; // Memory block size
			QWORD        XDAT:8;  // Memory block flags
		};
		struct
		{
			BYTE         RSV0[7];
			BYTE         TYPE:2;
			BYTE         PROT:5;
			BYTE         VALD:1;
		};
	};
};

void pmm_delete_link(LINEAR_MEMORY_BLOCK **root, LINEAR_MEMORY_BLOCK *blk, void (*freeNode)(LINEAR_MEMORY_BLOCK *));
void pmm_insert_link(LINEAR_MEMORY_BLOCK **root, LINEAR_MEMORY_BLOCK *blk, void (*freeNode)(LINEAR_MEMORY_BLOCK *));


#endif //SUPERNOVA_MEMBLK_H
