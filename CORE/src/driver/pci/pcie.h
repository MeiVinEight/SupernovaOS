#ifndef SUPERNOVA_PCIE_H
#define SUPERNOVA_PCIE_H

#include <acpi/acpi.h>

typedef struct _PCIE_SEGMENT_ADDRESS
{
	QWORD ECAM;
	WORD  PSEG;
	BYTE  SBUS;
	BYTE  EBUS;
	DWORD RSV0;
} PCIE_SEGMENT_ADDRESS;
#pragma pack(push, 4)
typedef struct _ACPI_MCFG
{
	ACPI_SDT_HEADER HEAD;
	DWORD RSV0[2];
	PCIE_SEGMENT_ADDRESS ECAM[];
} ACPI_MCFG;
#pragma pack(pop)

extern volatile ACPI_MCFG *volatile MCFG;

void setup_pcie_mcfg(ACPI_MCFG *mcfg);
void setup_pcie();

#endif
