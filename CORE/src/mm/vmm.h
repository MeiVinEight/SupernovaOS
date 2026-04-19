//
// Created by MeiVi on 2026/03/18.
//

#ifndef SUPERNOVA_VIRTMEM_H
#define SUPERNOVA_VIRTMEM_H

#include <types.h>

#define PAGE_4K 0
#define PAGE_2M 1
#define PAGE_1G 2

#define PAGE_4K_OFFSET 0xFFFULL
#define PAGE_2M_OFFSET 0x1FFFFFULL
#define PAGE_1G_OFFSET 0x3FFFFFFFULL

#define PAGING_PRESENT  (1ULL <<  0)
#define PAGING_WRITE    (1ULL <<  1)
#define PAGING_USER     (1ULL <<  2)
#define PAGING_PAGESIZE (1ULL <<  7)
#define PAGING_EXED     (1ULL << 63)

#define VMM_TYPE_FREE    0
#define VMM_TYPE_RESERVE 1
#define VMM_TYPE_COMMIT  2
#define VMM_TYPE_MASK    3

#define VMM_WRITE     1
#define VMM_EXECUTE   2

/**
 * F    F    F    F     8    0    0     0    0     0    0     0    0     0    0    0
 * 1111 1111 1111 1111 |1000 0000 0|000 0000 00|00 0000 000|0 0000 0000 |0000 0000 0000
 */

void setup_page_fault();
void setup_memory();
void vmm_free(void *ref, QWORD addr, QWORD pageCount);
QWORD vmm_alloc(void *root, QWORD *addr, QWORD pageCount, int align, DWORD type, DWORD protect, DWORD keep);
QWORD __stdcall alloc_physical_memory(QWORD pageCount, int align);
void __stdcall free_physical_memory(QWORD addr, QWORD pageCount);
void paging_attribute(QWORD virtAddr, QWORD attr);
void virtual_mapping(QWORD phyAddr, QWORD virtualAddr, QWORD pageCount, int pageType, QWORD attr);
QWORD physical_address(QWORD virtAddr);
void *heap_alloc(QWORD allocSize);
void heap_free(const volatile void *addr);
QWORD virtual_alloc(QWORD proc, QWORD *virtAddr, QWORD allocSize, DWORD allocType, DWORD protect);

#endif //SUPERNOVA_VIRTMEM_H
