#include <driver/ahci/ahci.h>
#include <mm/vmm.h>
#include <driver/pci/pci.h>
#include <stdio.h>
#include <core.h>
#include <timer/timer.h>
#include <intrinsic.h>
#include <driver/ahci/ata.h>

PCIE_AHCI_CONTROLLER *AHCI_CONTROLLER;

void setup_ahci_controller(PCI_EXPRESS_DEVICE *dev)
{
	PCI_DEVICE_VENDOR vendor;
	vendor.VENDOR = dev->configuration->vendor;
	vendor.DEVICE = dev->configuration->device;
	printf("PCI Express @ %016llX: %06X - ", (QWORD) dev->configuration, dev->configuration->class);
	const char *vendorName = pci_vendor_name(vendor.VENDOR);
	const char *deviceName = pci_device_name(vendor);
	if (vendorName && deviceName)
		printf("%s %s\n", vendorName, deviceName);
	else
		printf("%08lX\n", vendor.ID);

	PCIE_AHCI_CONTROLLER *controller = heap_alloc(sizeof(PCIE_AHCI_CONTROLLER));
	__memset(controller, 0, sizeof(PCIE_AHCI_CONTROLLER));
	controller->PCIE = *dev;
	controller->NEXT = AHCI_CONTROLLER;
	AHCI_CONTROLLER = controller;

	QWORD bar = pcie_cfg_get_base_address(&controller->PCIE, 5);
	pcie_enable_bus_master(&controller->PCIE);

	controller->AHBA = (AHCI_CONTROLLER_GENERAL *) core_mapping(bar);
	// Enable AHCI Mode
	controller->AHBA->AHCI = 1;
	for (DWORD pidx = 0; pidx < 32; pidx++)
	{
		if (!(controller->AHBA->PIMP & (1 << pidx)))
			continue;
		AHCI_CONTROLLER_PORT *port = controller->AHBA->PORT + pidx;
		DWORD ssts = port->STAT;
		BYTE ipm = (ssts >> 8) & 0x0F;
		BYTE det = (ssts >> 0) & 0x0F;
		// Check device present
		if (det != HBA_PORT_DET_PRESENT)
			continue;
		// Check device active
		if (ipm != HBA_PORT_IPM_ACTIVE)
			continue;
		// Reset port
		ahci_port_reset(port);
		if (port->SIGN != HBA_PORT_SIG_ATA)
			continue;
		// Allocate command and fis buffer
		QWORD phyAddr = alloc_physical_memory(1, 0);
		__memset((void *) core_mapping(phyAddr), 0, 0x1000);
		port->CMDA = phyAddr;
		port->FISA = phyAddr + 0x400;
		// Restart port
		port->CISS = 0;
		port->ERRO = -1;
		while (port->PCMD & HBA_PORT_CMD_CR) delay(1);
		port->PCMD |= HBA_PORT_CMD_FRE | HBA_PORT_CMD_ST;
		while ((port->STAT & 7) != 3) delay(1);

		SATA_STORAGE_DEVICE *sata = heap_alloc(sizeof(SATA_STORAGE_DEVICE));
		sata->SSSD.READ = ahci_ata_read;
		sata->CTRL = controller;
		sata->PORT = pidx;
		storage_insert(&sata->SSSD);
		/*
		phyAddr = alloc_physical_memory(1, 0);
		BYTE *buf = (BYTE *) core_mapping(phyAddr);
		__memset(buf, 0, 0x1000);
		DWORD cc = ahci_ata_operation(controller, pidx, 0, buf, 1, ATA_CMD_IDENTIFY_DEVICE);
		if (cc)
		{
			printf("AHCI ATA Operation: %lu\n", cc);
			continue;
		}
		BYTE (*swap)[2] = (BYTE (*)[2]) buf;
		for (int i = 0; i < 256; i++)
			swap[i][0] ^= swap[i][1] ^= swap[i][0] ^= swap[i][1];
		char *identify = (char *) (buf + 0x14);
		identify[0x14] = 0;
		printf("[%s] ", identify);
		identify = (char *) (buf + 0x36);
		identify[0x28] = 0;
		printf("[%s]\n", identify);
		*/
	}
}
void ahci_port_reset(AHCI_CONTROLLER_PORT *port)
{
	// It must first clear PxCMD.ST,
	DWORD cmd = port->PCMD;
	cmd &= ~HBA_PORT_CMD_CLO;
	cmd &= ~HBA_PORT_CMD_ST;
	cmd &= ~HBA_PORT_CMD_FRE;
	port->PCMD = cmd;
	// and wait for PxCMD.CR to clear to 0 before re-initiaizing communication.
	DWORD wait = 500;
	while ((((volatile AHCI_CONTROLLER_PORT *) port)->PCMD & HBA_PORT_CMD_CR) && (wait--)) delay(1);
	wait++;
	if (wait)
		return;

	// However, if PxCMD.CR does not clear withing a reasonable time (500ms),
	// it may assume the interface is in a hung condition and may continue with issuing the port reset.
	// Software cause a port reset (COMRESET) by writing 1 to the PxSCTL.DET field to invoke a
	// COMRESET on the interface and start a re-establishment of Phy layer communications.
	port->CTRL = (port->CTRL & ~0x0F) | 1;
	// Software shall wait at list 1ms before clearing PxSCTL.DET to 0; this ensures that at least one COMRESET
	// signal is send over the interface.
	delay(1);
	port->CTRL &= ~0x0F;
}

