//
// Created by MeiVi on 2026/03/11.
//

#ifndef SUPERNOVA_ACPI_H
#define SUPERNOVA_ACPI_H

#include  <types.h>

#define ACPI_SIGNATURE_MADT 0x43495041 // APIC
#define ACPI_SIGNATURE_MCFG 0x4746434D // MCFG

typedef struct _ACPI_RSDP
{
	char SIGN[8];
	BYTE CHKS;
	char OEMS[6];
	BYTE REVI;
	DWORD RSDT;
	// ACPI 2.0
	DWORD LENH;
	QWORD XSDT;
	BYTE XCHK;
	BYTE RSV0[3];
} ACPI_RSDP;
#pragma pack(push, 4)
typedef struct _ACPI_ADDRESS
{
	BYTE  SPID; // Space ID, 0: System Memory, 1: System I/O
	BYTE  WIDT; // Register bit width
	BYTE  OFST; // Register bit offset
	BYTE  RSV0;
	QWORD ADDR;
} ACPI_ADDRESS;
#pragma pack(pop)
typedef struct _ACPI_SDT_HEADER
{
	BYTE  SIGN[4];
	DWORD LENG;
	BYTE  RVSN;
	BYTE  CSUM;
	BYTE  OEMD[6];
	BYTE  OEMT[8];
	DWORD OEMR;
	DWORD CRTI;
	DWORD CRTR;
} ACPI_SDT_HEADER;
typedef struct _ACPI_RSDT
{
	ACPI_SDT_HEADER HEAD;
	DWORD TABLE[];
} ACPI_RSDT;
#pragma pack(push, 4)
typedef struct _ACPI_XSDT
{
	ACPI_SDT_HEADER HEAD;
	QWORD TABLE[];
} ACPI_XSDT;
#pragma pack(pop)

void setup_acpi();
ACPI_RSDP *acpi_find_rsdp();

#endif //SUPERNOVA_ACPI_H
