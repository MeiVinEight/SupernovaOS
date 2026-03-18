//
// Created by MeiVi on 2026/03/11.
//

#ifndef SUPERNOVA_PE32X_H
#define SUPERNOVA_PE32X_H

#include <types.h>

typedef struct _IMAGE_DOS_HEADER
{
	WORD A5D4; // MZ
	WORD CBLP; // Last page byte count
	WORD PCNT; // Page count
	WORD CRLC; // Relocation count
	WORD CHDR; // Header size (in 0x10)
	WORD MINM; // Min memory
	WORD MAXM; // Max memory
	WORD IISS; // Initial SS
	WORD IISP; // Initial SP
	WORD CSUM; // Checksum
	WORD IIIP; // Initial IP
	WORD IICS; // Initial CS
	WORD ARLC; // Relocation Table Address
	WORD OVNO; // Overlap Number
	WORD RSV0[4];
	WORD OEMX; // OEM ID
	WORD OEMI; // OEM INFO
	WORD RSV1[10];
	DWORD ANEW;// PE HEADER OFFSET
} IMAGE_DOS_HEADER;

#endif //SUPERNOVA_PE32X_H
