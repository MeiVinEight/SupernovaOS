#pragma once

#include <driver/ahci/ahci.h>
#include <driver/disk/disk.h>

#define ATA_CMD_READ_DMA_EX      0x25
#define ATA_CMD_IDENTIFY_DEVICE  0xEC

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ  0x08

#define ATA_SECTOR_PRE_PRDT      8
#define ATA_MAX_SECTOR_PRE_OP 1024

typedef struct _ATA_IDENTIFY_DEVICE
{
	WORD GCH;
} ATA_IDENTIFY_DEVICE;
typedef struct _SATA_STORAGE_DEVICE
{
	STANDARD_STORAGE_DEVICE  SSSD;
	PCIE_AHCI_CONTROLLER    *CTRL;
	BYTE                     PORT;
} SATA_STORAGE_DEVICE;

DWORD ahci_ata_operation(PCIE_AHCI_CONTROLLER *controller, BYTE portId, QWORD lba, void *buffer, DWORD count, BYTE cmd);
QWORD ahci_ata_read(STANDARD_STORAGE_DEVICE *device, void *buf, QWORD lba, DWORD count);