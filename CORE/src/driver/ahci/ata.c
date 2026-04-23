#include <driver/ahci/ata.h>
#include <intrinsic.h>
#include <core.h>
#include <timer/timer.h>
#include <mm/vmm.h>

DWORD ahci_ata_operation(PCIE_AHCI_CONTROLLER *controller, BYTE portId, QWORD lba, void *buffer, DWORD count, BYTE cmd)
{
	// Check page boundary
	QWORD buffAddr = (QWORD) buffer;
	QWORD startPage = buffAddr >> 12;
	QWORD endPage = (buffAddr + ((QWORD) count << 9)) >> 12;
	if ((buffAddr & 0xFFF) && (startPage != endPage))
	{
		// crossing page boundary and is not aligned
		return 1;
	}
	AHCI_CONTROLLER_PORT *port = controller->AHBA->PORT + portId;
	if (port->PTFD & (ATA_DEV_BUSY | ATA_DEV_DRQ))
		return 2;

	async_lock(controller->LOCK + portId);
	// Clear pending interrupt bits
	port->INTS = -1;
	DWORD slot = 0;
	AHCI_COMMAND_HEADER *cmdh = (AHCI_COMMAND_HEADER *) core_mapping(port->CMDA);
	__memset(cmdh, 0, sizeof(AHCI_COMMAND_HEADER));
	cmdh += slot;
	// Command FIS size
	cmdh->CFIL = 5; // FIS DWORD COUNT
	// Read
	cmdh->WRIT = 0;
	// Clear busy
	//cmdh->CBOK = 1;
	// PRDT entries count
	cmdh->RDTL = (((QWORD) count) + (ATA_SECTOR_PRE_PRDT - 1)) / ATA_SECTOR_PRE_PRDT;

	cmdh->CTBA = port->FISA + 0x100;
	AHCI_COMMAND_TABLE *tbl = (AHCI_COMMAND_TABLE *) core_mapping(cmdh->CTBA);
	__memset(tbl, 0, sizeof(AHCI_COMMAND_TABLE) + (sizeof(AHCI_PRDT_ENTRY) * cmdh->RDTL));
	// Default 4KiB (8 sector) pre PRDT
	QWORD buf = (QWORD) buffer;
	DWORD cnt1 = count;
	DWORD i = 0;
	while (cnt1)
	{
		DWORD cnt = (cnt1 < ATA_SECTOR_PRE_PRDT) ? cnt1 : ATA_SECTOR_PRE_PRDT;
		tbl->PRDT[i].DATA = physical_address(buf);
		tbl->PRDT[i].BCNT = (cnt << 9) - 1;
		// Interrupt on Completion
		// tbl->PRDT[i].IONC = 1;
		cnt1 -= cnt;
		buf += cnt << 9;
		i++;
	}

	// Setup Command
	AHCI_FIS_REG_H2D *fis = (AHCI_FIS_REG_H2D *) tbl->CFIS;
	fis->TYPE = 0x27; // Register FIS - host to device
	fis->CORC = 1; // Command
	fis->CMMD = cmd;
	fis->FEA0 = 1; // DMA

	fis->LBA0 = (lba >>  0) & 0xFF;
	fis->LBA1 = (lba >>  8) & 0xFF;
	fis->LBA2 = (lba >> 16) & 0xFF;
	fis->DEVC = (1 << 6); // LBA Mode

	fis->LBA3 = (lba >> 24) & 0xFF;
	fis->LBA4 = (lba >> 32) & 0xFF;
	fis->LBA5 = (lba >> 40) & 0xFF;

	fis->CONT = count;

	port->CISS = 1 << slot;

	// Wait for completion
	while ((port->CISS & (1 << slot)) && !(port->INTS & HBA_PORT_IS_TFES)) delay(1);
	async_unlock(controller->LOCK + portId);
	if (port->INTS & HBA_PORT_IS_TFES)
		return 3;
	return 0;
}
QWORD ahci_ata_read(STANDARD_STORAGE_DEVICE *device, void *buf, QWORD lba, DWORD count)
{
	if (!device)
		return 1;
	if (!buf)
		return 2;
	if (!count)
		return 0;

	SATA_STORAGE_DEVICE *dev = (SATA_STORAGE_DEVICE *) device;

	// Check page boundary
	QWORD buffAddr = (QWORD) buf;
	QWORD startPage = buffAddr >> 12;
	QWORD endPage = (buffAddr + ((QWORD) count << 9)) >> 12;
	if ((buffAddr & 0xFFF) && (startPage != endPage))
	{
		BYTE *buffer = buf;
		QWORD alignedAddr = 0;
		alignedAddr = alloc_physical_memory(1, 0);
		void *bufAlign = (void *) core_mapping(alignedAddr);
		while (count)
		{
			DWORD cnt = count;
			if (cnt > 8)
				cnt = 8;
			DWORD cc = ahci_ata_operation(dev->CTRL, dev->PORT, lba, bufAlign, cnt, ATA_CMD_READ_DMA_EX);
			if (cc)
				return cc;
			__memcpy(buffer, bufAlign, cnt << 9);
			count -= cnt;
			buffer += cnt << 9;
		}
	}
	else
	{
		BYTE *buffer = buf;
		while (count)
		{
			DWORD cnt = count;
			if (cnt > ATA_MAX_SECTOR_PRE_OP)
				cnt = ATA_MAX_SECTOR_PRE_OP;
			// Assume page mapped
			(void) *((volatile BYTE *) buffer); // Make sure physical page allocated
			DWORD cc = ahci_ata_operation(dev->CTRL, dev->PORT, lba, buffer, cnt, ATA_CMD_READ_DMA_EX);
			if (cc)
				return cc;
			buffer += cnt << 9;
			count -= cnt;
		}
	}
	return 0;
}