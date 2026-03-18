#include <uefi.h>
#include <ahci.h>

DWORD check_type(HBA_PORT *port)
{
	DWORD ssts = port->ssts;

	BYTE ipm = (ssts >> 8) & 0x0F;
	BYTE det = (ssts >> 0) & 0x0F;

	// Check device present
	if (det != HBA_PORT_DET_PRESENT)
		return AHCI_DEVICE_NULL;
	// Check device active
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return AHCI_DEVICE_NULL;

	switch (port->sig)
	{
		case SATA_SIG_ATA:   return AHCI_DEVICE_SATA;
		case SATA_SIG_ATAPI: return AHCI_DEVICE_SATAPI;
		case SATA_SIG_SEMB:  return AHCI_DEVICE_SEMB;
		case SATA_SIG_PM:    return AHCI_DEVICE_PM;
		default:             return AHCI_DEVICE_NULL;
	}
}

// Stop command engine
void stop_cmd(HBA_PORT *port)
{
	// Clear ST (bit 0)
	port->cmd &= ~HBA_PORT_CMD_ST;

	// Clear FRE (bit4);
	port->cmd &= ~HBA_PORT_CMD_FRE;

	// Wait until FR (bit14), CR (bit15) are cleared;
	while (port->cmd & (HBA_PORT_CMD_FR | HBA_PORT_CMD_CR));
}
DWORD start_cmd(HBA_PORT *port)
{
	// Use 2 pages
	//QWORD addr = 0x00008000;
	// Stop command engine
	stop_cmd(port);

	// Start command engine
	// Wait until CR (bit15) is cleared
	while (port->cmd & HBA_PORT_CMD_CR);

	// Set FRE (bit4) and ST (bit0)
	DWORD cmd = port->cmd;
	cmd |= HBA_PORT_CMD_FRE;
	cmd |= HBA_PORT_CMD_ST;
	cmd |= HBA_PORT_CMD_SPIN_UP;
	port->cmd = cmd;
	int spin = 0;
	while (spin++ < 1000000)
	{
		if ((port->ssts & 7) == 3)
		{
			// Success
			return 0;
		}
	}
	OutputText("AHCI LINK DOWN\r\n");
	return 2;
}

DWORD ahci_command(HBA_PORT *port, QWORD sector, WORD count, void *buffer, DWORD cmd)
{
	if (!count) return 0;
	if (count > 128) return 1;

	// Clear pending interrupt bits
	port->is = -1;
	// Slot is used for multi thread
	int slot = 0;// ahci_find_cmdslot(port);
	if (slot == -1)
		return 2;

	HBA_CMD_HEADER *cmdh = (HBA_CMD_HEADER *) ((port->clb | ((QWORD) port->clbu << 32)));
	cmdh += slot;
	// Command FIS size
	cmdh->cfl = 5; // sizeof(FIS_REG_H2D) / sizeof(DWORD)
	// Read from device
	cmdh->w = 0;
	// cmdh->c = 1;
	// cmdh->p = 1;
	// PRDT entries count
	cmdh->prdtl = ((count - 1) >> 4) + 1; // UPPER BOUND (count / 4)

	HBA_CMD_TBL *tbl = (HBA_CMD_TBL *) ((cmdh->ctba | ((QWORD) cmdh->ctbau << 32)));
	memset(tbl, 0, sizeof(HBA_CMD_TBL) + (cmdh->prdtl * sizeof(HBA_PRDT_ENTRY)));
	// 8KiB (16 sectors) per PRDT
	QWORD buf = (QWORD) buffer;
	WORD count1 = count;
	DWORD i = 0;
	while (count1)
	{
		DWORD cnt = (count1 < 16) ? count1 : 16;
		tbl->prdt_entry[i].dba = (DWORD) buf;
		tbl->prdt_entry[i].dbau = buf >> 32;
		tbl->prdt_entry[i].dbc = (cnt << 9) - 1;
		tbl->prdt_entry[i].i = 0; // Right or Wrong ?
		count1 -= cnt; // 16 sectors
		buf += ((QWORD) cnt << 9); // 8KiB
		i++;
	}

	// Setup command
	FIS_REG_H2D *fis = (FIS_REG_H2D *) tbl->cfis;
	memset(fis, 0, sizeof(FIS_REG_H2D));

	fis->fis_type = 0x27; // Register FIS - host to device
	fis->c = 1; // Command
	fis->command = cmd;
	fis->featurel = 1; /* dma */

	fis->lba0 = (sector >> 0) & 0xFF;
	fis->lba1 = (sector >> 8) & 0xFF;
	fis->lba2 = (sector >> 16) & 0xFF;
	fis->device = 1 << 6; // LBA MODE
	fis->lba3 = (sector >> 24) & 0xFF;
	fis->lba4 = (sector >> 32) & 0xFF;
	fis->lba5 = (sector >> 40) & 0xFF;

	fis->countl = (count >> 0) & 0xFF;
	fis->counth = (count >> 8) & 0xFF;

	// The below loop waits until the port is no longer busy bufore issuing a new command
	int spin = 0; // Spin lock timeout counter
	while ((port->tfd & (ATA_DEV_DRQ | ATA_DEV_BUSY)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		return 3;
	}

	port->ci = 1 << slot;

	// Wait for completion
	while ((port->ci & (1 << slot)) && !(port->is & HBA_PxIS_TFES));
	if (port->is & HBA_PxIS_TFES)
	{
		return 4;
	}
	return 0;
}