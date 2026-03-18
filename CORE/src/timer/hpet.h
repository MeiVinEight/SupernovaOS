//
// Created by MeiVi on 2026/03/14.
//

#ifndef SUPERNOVA_HPET_H
#define SUPERNOVA_HPET_H

#include <acpi/acpi.h>

#define HPET_GCFR_ENABLE (1ULL << 0)
#define HPET_GCFR_LEG_RT (1ULL << 1)

typedef struct _ACPI_HPET
{
	ACPI_SDT_HEADER HEAD;
	BYTE HREV; // Hardware revision id
	BYTE CCMP:5; // Comparator count
	BYTE CNTS:1; // Counter size
	BYTE RSV0:1;
	BYTE REPL:1; // Legacy replacement
	WORD PVID; // PCI Vendor ID
	ACPI_ADDRESS ADDR;
	BYTE TNUM; // HPET Number
	BYTE MNTL; // Min tick
	BYTE MNTH; // Min tick high 8 bit
	BYTE PROT; // Page protection
} ACPI_HPET;
typedef struct _HPET_REGISTER
{
	volatile QWORD GCIR; // General Capabilities and ID Register
	volatile QWORD RSV0;
	volatile QWORD GCFR; // General Configuration Register
	volatile QWORD RSV1;
	volatile QWORD GISR; // General Interrupt Status Register
	volatile QWORD RSV2[25];
	volatile QWORD MCVR; // Main Counter Value Register
	volatile QWORD RSV3;
	volatile QWORD CCR0; // Timer 0 Configuration and Capability Register
	volatile QWORD CVR0; // Timer 0 Comparator Value Register
	volatile QWORD IRR0; // Timer 0 FSB Interrupt Route Register
	volatile QWORD RSV4;
	volatile QWORD CCR1; // Timer 1 Configuration and Capability Register
	volatile QWORD CVR1; // Timer 1 Comparator Value Register
	volatile QWORD IRR1; // Timer 1 FSB Interrupt Route Register
	volatile QWORD RSV5;
	volatile QWORD CCR2; // Timer 2 Configuration and Capability Register
	volatile QWORD CVR2; // Timer 2 Comparator Value Register
	volatile QWORD IRR2; // Timer 2 FSB Interrupt Route Register
	volatile QWORD RSV6;
	volatile QWORD RSV7[0x54];
} HPET_REGISTER;

extern ACPI_HPET HPET_TABLE;

void setup_hpet(ACPI_HPET *hpet);
QWORD hpet_query_frequency();
QWORD hpet_get_counter();

#endif //SUPERNOVA_HPET_H
