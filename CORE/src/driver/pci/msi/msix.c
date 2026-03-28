#include <driver/pci/pcie.h>
#include <driver/pci/msi/msi.h>
#include <driver/pci/msi/msix.h>
#include <core.h>
#include <console.h>
#include <interrupt/apic.h>

DWORD __stdcall pcie_setup_msix(volatile PCI_EXPRESS_DEVICE *device, DWORD intx)
{
	if (intx > 0xFF)
		return 0;

	// Find the MSI-X capability structure
	volatile PCI_EXPRESS_CAPABILITY_MSI_X *cap = (PCI_EXPRESS_CAPABILITY_MSI_X *) pcie_capability(device, PCIE_CAID_MSIX);
	if (!cap)
		return 0;
	simple_output("PCI Express @ ");
	simple_output_address((QWORD) cap, 16);
	simple_output(": MSI-X Table Size = ");
	simple_output_address(cap->TABS, 4);
	outchar('\n');

	// Disable MSI-X temporarily by clearing the MSI-X enable bit (bit 15)
	cap->MXEN = 0;

	// Extract BAR number (lower 3 bits) and offset (remaining bits)
	DWORD tableBir = cap->TBIR;
	DWORD pbaBir = cap->PBIR;

	// Make sure the BAR is valid before using it
	if (tableBir >= pcie_bar_cound(device) || pbaBir >= pcie_bar_cound(device))
		return 0;

	// Get bars
	QWORD tableBar = pcie_cfg_get_base_address(device, tableBir);
	QWORD pbaBar = pcie_cfg_get_base_address(device, pbaBir);

	// Map the MSI-X table and PBA with proper page alignment handling
	// PASS

	if (!tableBar || !pbaBar)
		return 0;

	tableBar += (cap->TOFF << 3);
	pbaBar += (cap->POFF << 3);

	// Build the MSI-X address and data
    // Populate the vector table entry
	volatile PCI_EXPRESS_MSIX_TABLE_ENTRY *msixTable = (PCI_EXPRESS_MSIX_TABLE_ENTRY *) core_mapping(tableBar);
	msixTable[0].ADDR = MSI_ADDRESS_BASE | apic_current_id();
	msixTable[0].DATA = APIC_ICR_DELIVERY_FIXED | intx;
	msixTable[0].CTRL = 0;

	// Re-enable MSI-X
	cap->MXEN = 1;

	// Clear any pending interrupts for this vector
	clear_msix_pending_bit((DWORD *) core_mapping(pbaBar), 0);

	return 1;
}
void clear_msix_pending_bit(volatile DWORD *pba, DWORD entryIdx)
{
	DWORD dwOff = entryIdx >> 5;
	DWORD bitOff = entryIdx & 0x1F;
	pba[dwOff] = pba[dwOff] & (~(1 << bitOff));
}