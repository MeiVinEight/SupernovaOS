#include <pci.h>
#include <uefi.h>
#include <pcie.h>

#define PCI_COMMAND         0x04	/* 16 bits */
#define PCI_BASE_ADDRESS_5	0x24	/* 32 bits */

#define PCI_COMMAND_MEMORY  0x2	/* Enable response in Memory space */
#define PCI_COMMAND_MASTER  0x4	/* Enable bus mastering */

void *pci_enable_mmio(DWORD device, DWORD addr)
{
	__outdword(0xCF8, device + addr);
	QWORD bar = __indword(0xCFC);
	// Check BAR PIO Mode
	if (bar & 1)
		return 0;
	bar &= ~0xF; // Clear lower 4 bits
	// Use base as mmio address, write to BAR
	//__outdword(0xCF8, device + addr);
	//__outdword(0xCFC, base | (bar & 0xF))
	// Read Command Register
	__outdword(0xCF8, device + PCI_COMMAND);
	WORD cmd = __inword(0xCFC);
	// Set Memory Space Enable bit
	cmd |= PCI_COMMAND_MEMORY;
	// Write to command register
	__outdword(0xCF8, device + PCI_COMMAND);
	__outword(0xCFC, cmd);
	return (void *) bar;
}
void *pcie_enable_mmio(volatile PCI_CONFIGURATION_SPACE *conf, DWORD addr)
{
	QWORD bar = conf->address[addr];
	// Check BAR PIO Mode
	if (bar & 1)
		return 0;
	bar &= ~0xFULL; // Clear lower 4 bits
	// Use base as mmio address, write to BAR
	//conf->address[addr] = base | (bar & 0xF);
	// Read Command Register
	WORD cmd = conf->command;
	// Set Memory Space Register
	cmd |= PCI_COMMAND_MEMORY;
	// Write to command register
	conf->command = cmd;
	return (void *) bar;
}