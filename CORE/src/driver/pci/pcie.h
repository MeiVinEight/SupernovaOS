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
typedef struct _PCI_CONFIGURATION_INTERRUPT
{
	BYTE line;
	BYTE pin;
	BYTE grant;
	BYTE latency;
} PCI_CONFIGURATION_INTERRUPT;
typedef struct _PCI_CONFIGURATION_SPACE
{
	WORD vendor;
	WORD device;
	WORD command;
	WORD status;
	DWORD revision:8;
	DWORD class:24;
	BYTE cache;
	BYTE latency;
	BYTE type;
	BYTE bist; // Built-In Self-Test
	DWORD address[6];
	DWORD cardbus;
	DWORD subsystem;
	DWORD expansion;
	BYTE capabilities;
	BYTE resvered0[7];
	PCI_CONFIGURATION_INTERRUPT interrupt;
} PCI_CONFIGURATION_SPACE;
typedef struct _PCIE_DEVICE
{
	DWORD offset:8;
	DWORD function:3;
	DWORD device:5;
	DWORD bus:8;
	DWORD reserve:8;
	volatile PCI_CONFIGURATION_SPACE *configuration;
} PCIE_DEVICE;

extern volatile ACPI_MCFG *volatile MCFG;

void setup_pcie_mcfg(ACPI_MCFG *mcfg);
void setup_pcie();
QWORD pcie_cfg_get_base_address(volatile PCIE_DEVICE *device, DWORD addrIdx);

#endif
