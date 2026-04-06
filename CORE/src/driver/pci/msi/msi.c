#include <driver/pci/msi/msi.h>
#include <interrupt/apic.h>

DWORD pcie_setup_msi(PCI_EXPRESS_DEVICE *device, DWORD intx)
{
	if (intx > 0xFF)
		return 0;

	// Find the MSI-X capability structure
	volatile PCI_EXPRESS_CAPABILITY_MSI64 *cap = (PCI_EXPRESS_CAPABILITY_MSI64 *) pcie_capability(device, PCIE_CAID_MSI);
	if (!cap)
		return 0;;

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