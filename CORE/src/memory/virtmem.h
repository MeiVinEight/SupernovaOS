//
// Created by MeiVi on 2026/03/18.
//

#ifndef SUPERNOVA_VIRTMEM_H
#define SUPERNOVA_VIRTMEM_H

#include <types.h>

#define PAGE_4K 0
#define PAGE_2M 1
#define PAGE_1G 2

#define PAGING_PRESENT  0x001
#define PAGING_WRITE    0x002
#define PAGING_USER     0x004
#define PAGING_PAGESIZE 0x080

#define PA_WRITE 0x01
#define PA_USER  0x02

/**
 * F    F    F    F     8    0    0     0    0     0    0     0    0     0    0    0
 * 1111 1111 1111 1111 |1000 0000 0|000 0000 00|00 0000 000|0 0000 0000 |0000 0000 0000
 */

void setup_memory();
QWORD __stdcall alloc_physical_memory(QWORD *pageCount, int align, int continu);
void __stdcall free_physical_memory(QWORD addr, QWORD pageCount);
void virtual_mapping(QWORD phyAddr, QWORD virtualAddr, QWORD pageCount, int pageType, QWORD attr);
QWORD physical_address(QWORD virtAddr);
void *heap_alloc(QWORD allocSize);
void heap_free(const volatile void *addr);

#endif //SUPERNOVA_VIRTMEM_H
