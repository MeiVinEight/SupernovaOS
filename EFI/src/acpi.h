#pragma once

#define ACPI_SIGNATURE_MCFG 0x4746434D // MCFG

#include <intrinsic.h>

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
typedef struct _ACPI_SDT_HEADER
{
	BYTE SIGN[4];
	DWORD LENG;
	BYTE RVSN;
	BYTE CSUM;
	BYTE OEMD[6];
	BYTE OEMT[8];
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