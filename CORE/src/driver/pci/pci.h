//
// Created by MeiVi on 2026/03/10.
//

#ifndef SUPERNOVA_PCI_H
#define SUPERNOVA_PCI_H

#include <intrinsic.h>

// PCI configuration space constants
#define PCI_CONFIG_ADDRESS           0xCF8
#define PCI_CONFIG_DATA              0xCFC

// PCI configuration register offsets
#define PCI_OFFSET_VENDOR            0x00
#define PCI_OFFSET_COMMAND           0x04
#define PCI_OFFSET_STATUS            0x06
#define PCI_OFFSET_REVISION          0x08
#define PCI_OFFSET_HEADER_TYPE       0x0E
#define PCI_OFFSET_BAR0              0x10
#define PCI_OFFSET_SUBSYSTEM         0x2C

// Command register bits
#define PCI_COMMAND_IO_SPACE         (1 << 0)
#define PCI_COMMAND_MEMORY_SPACE     (1 << 1)
#define PCI_COMMAND_BUS_MASTER       (1 << 2)

typedef union _PCI_DEVICE_ADDRESS
{
	struct
	{
		DWORD ZRO:2;
		DWORD REG:6;
		DWORD FUN:3;
		DWORD DVC:5;
		DWORD BUS:8;
		DWORD RSV:7;
		DWORD ENABLE:1;
	};
	volatile DWORD address;
} PCI_DEVICE_ADDRESS;
typedef union _PCI_DEVICE_VENDOR
{
	struct
	{
		volatile WORD VENDOR;
		volatile WORD DEVICE;
	};
	volatile DWORD ID;
} PCI_DEVICE_VENDOR;
typedef union _PCI_DEVICE_SUBSYSTEM
{
	struct
	{
		WORD subvendor;
		WORD subsystem;
	};
	DWORD value;
} PCI_DEVICE_SUBSYSTEM;
typedef struct _PCI_DEVICE
{
	PCI_DEVICE_ADDRESS address;
	PCI_DEVICE_VENDOR vendor;
	DWORD type;
	PCI_DEVICE_SUBSYSTEM subsystem;
} PCI_DEVICE;

DWORD pci_read_config_word(DWORD deviceId, DWORD offset);
DWORD pci_read_config_dword(DWORD deviceId, DWORD offset);
void pci_write_config_word(DWORD deviceId, DWORD offset, DWORD value);
PCI_DEVICE_VENDOR pci_read_register_vendor(PCI_DEVICE_ADDRESS address);
DWORD pci_read_register_class(PCI_DEVICE_ADDRESS address);
DWORD pci_read_register_command(DWORD deviceId);
void pci_write_register_command(DWORD deviceId, DWORD value);
DWORD pci_read_register_header_type(PCI_DEVICE_ADDRESS address);
PCI_DEVICE_SUBSYSTEM pci_read_register_subsystem(PCI_DEVICE_ADDRESS address);
void pci_enable(PCI_DEVICE_ADDRESS deviceId);
void setup_pci();

#endif //SUPERNOVA_PCI_H
