#include <acpi/acpi.h>
#include <acpi/fadt.h>
#include <core.h>
#include <timer/hpet.h>
#include <interrupt/apic.h>
#include <driver/pci/pcie.h>

void setup_acpi()
{
	ACPI_RSDP *rsdp = acpi_find_rsdp();
	if (!rsdp)
		return;

	QWORD sign = 0;
	volatile DWORD *dwSign = (DWORD *) &sign;
	char *signName = (char *) dwSign;
	ACPI_RSDT *rsdt = (ACPI_RSDT *) core_mapping(rsdp->RSDT);
	DWORD entryCount = (rsdt->HEAD.LENG - sizeof(ACPI_SDT_HEADER)) >> 2;
	for (DWORD enityIdx = 0; enityIdx < entryCount; enityIdx++)
	{
		DWORD entryPhyAddr = rsdt->TABLE[enityIdx];
		ACPI_SDT_HEADER *entry = (ACPI_SDT_HEADER *) core_mapping(entryPhyAddr);
		*dwSign = *((DWORD *) entry->SIGN);

		if (*dwSign == 0x54455048) // HPET
			setup_hpet((ACPI_HPET *) entry);
		if (*dwSign == ACPI_SIGNATURE_MADT)
			setup_madt((ACPI_MADT *) entry);
		if (*dwSign == ACPI_SIGNATURE_MCFG)
			setup_pcie_mcfg((ACPI_MCFG *) entry);
		if (*dwSign == ACPI_SIGNATURE_FADT)
			setup_fadt((ACPI_FADT *) entry);
	}
	/*
	ACPI_XSDT *xsdt = (ACPI_XSDT *) core_mapping(rsdp->XSDT);
	entryCount = (xsdt->HEAD.LENG - sizeof(ACPI_SDT_HEADER)) >> 3;
	for (DWORD enityIdx = 0; enityIdx < entryCount; enityIdx++)
	{
		QWORD entryPhyAddr = xsdt->TABLE[enityIdx];
		ACPI_SDT_HEADER *entry = (ACPI_SDT_HEADER *) core_mapping(entryPhyAddr);
		*dwSign = *((DWORD *) entry->SIGN);
		simple_output(signName);
		outchar('\n');
	}
	*/
}
ACPI_RSDP *acpi_find_rsdp()
{
	if (SYSTEM_TABLE->RSDP)
		return (ACPI_RSDP *) core_mapping(SYSTEM_TABLE->RSDP);

	QWORD signature = 0x0052545020445352; // RSD PTR
	QWORD (*biosMemory)[2] = ((QWORD (*)[2]) core_mapping(0x000E0000));
	for (; biosMemory < ((QWORD (*)[2]) core_mapping(0x00100000)); biosMemory++)
	{
		if ((*biosMemory)[0] == signature)
			return  (ACPI_RSDP *) biosMemory;
	}

	return 0;
}
