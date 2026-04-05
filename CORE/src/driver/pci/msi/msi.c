#include <driver/pci/msi/msi.h>
#include <interrupt/apic.h>
#include <console.h>

DWORD pcie_setup_msi(PCI_EXPRESS_DEVICE *device, DWORD intx)
{
	if (intx > 0xFF)
		return 0;

	// Find the MSI-X capability structure
	volatile PCI_EXPRESS_CAPABILITY_MSI64 *cap = (PCI_EXPRESS_CAPABILITY_MSI64 *) pcie_capability(device, PCIE_CAID_MSI);
	if (!cap)
		return 0;

	simple_output("PCI Express @ ");
	simple_output_address((QWORD) device->configuration, 16);
	simple_output(": ");
	if (cap->AC64)
		simple_output_number(64);
	else
		simple_output_number(32);
	simple_output(" | Multiple Message = ");
	simple_output_number(1 << cap->MMCA);
	if (cap->PVMC)
		simple_output(" | PVM");
	outchar('\n');

	cap->MSIE = 1;
	cap->MMEN = 0;
	QWORD msiAddr = MSI_ADDRESS_BASE | apic_current_id();
	DWORD msiData = APIC_ICR_DELIVERY_FIXED | intx;
	if (cap->AC64)
	{
		cap->ADDR = msiAddr;
		cap->DATA = msiData;
		if (cap->PVMC)
			cap->MASK = 0, cap->PEND = 0;
	}
	else
	{
		volatile PCI_EXPRESS_CAPABILITY_MSI32 *cap32 = (PCI_EXPRESS_CAPABILITY_MSI32 *) cap;
		cap32->ADDR = msiAddr;
		cap32->DATA = msiData;
		if (cap32->PVMC)
			cap32->MASK = 0, cap32->PEND = 0;
	}
	return 1;
}