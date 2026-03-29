#include <acpi/fadt.h>
#include <intrinsic.h>
#include <console.h>

void setup_fadt(volatile ACPI_FADT *fadt)
{
	simple_output("ACPI FADT @ ");
	simple_output_address((QWORD) fadt, 16);
	outchar('\n');

	simple_output("SCI Interrupt ");
	simple_output_address(fadt->SCIV, 2);
	outchar('\n');

	simple_output("Enable ACPI\n");

	if (!fadt->SMIC)
	{
		simple_output("NO SMI Command\n");
		return;
	}
	if (!fadt->ACPE)
	{
		simple_output("NO ACPI Enable Command\n");
		return;
	}
	__outbyte(fadt->SMIC, fadt->ACPE);
	while (!(__inword(fadt->PM1X.CNTA) & FADT_PM1_ST_SCI_EN)) __halt();
	if (fadt->PM1X.CNTB)
		while (!(__inword(fadt->PM1X.CNTB) & FADT_PM1_ST_SCI_EN)) __halt();

	simple_output("POWER BUTTON\n");

	DWORD len = fadt->P1EL >> 1;
	DWORD enReg1a = fadt->PM1X.EVTA + len;
	DWORD enReg1b = fadt->PM1X.EVTB;
	if (enReg1b)
		enReg1b += len;

	__outword(enReg1a, FADT_PM1_EN_PWRBTN_EN);
	if (enReg1b)
		__outword(enReg1b, FADT_PM1_EN_PWRBTN_EN);
}