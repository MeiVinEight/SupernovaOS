#include <acpi/fadt.h>
#include <interrupt/apic.h>
#include <interrupt/interrupt.h>
#include <acpi/dsdt.h>
#include <intrinsic.h>
#include <console.h>
#include <core.h>
#include <stdio.h>

volatile ACPI_FADT *volatile FIXED_ACPI_TABLE = 0;

volatile QWORD FADT_STATUS = 0;
volatile DWORD PM1_SLP_TYPE5_A = 0;
volatile DWORD PM1_SLP_TYPE5_B = 0;

void setup_fadt_s5()
{
	// Find _S5_
	QWORD dsdtAddr = FIXED_ACPI_TABLE->DSDT;
	if (FIXED_ACPI_TABLE->HEAD.LENG >= 148 && FIXED_ACPI_TABLE->XDSD)
		dsdtAddr = FIXED_ACPI_TABLE->XDSD;
	ACPI_DSDT *dsdt = (ACPI_DSDT *) core_mapping(dsdtAddr);
	volatile BYTE *volatile data = dsdt->DATA;
	DWORD dsdtLen = dsdt->HEAD.LENG;
	while (dsdtLen && *((DWORD *) data) != DSDT_SIGNATURE_S5) data++, dsdtLen--;
	if (!dsdtLen)
		return;
	if ((data[-1] != 0x08) && ((data[-2] != 0x08) && (data[-1] != '\\')))
		return;

	// Skip "_S5_" and PackageOp
	data += 5;
	// Skip Length
	data += ((*data) >> 6) + 2;
	if (*data == 0x0A) data++;
	PM1_SLP_TYPE5_A = (*data) << 10;
	data++;
	if (*data == 0x0A) data++;
	PM1_SLP_TYPE5_B = (*data) << 10;
	data++;
	FADT_STATUS |= FADT_STAT_S5;
}
void system_control_interrupt(INTERRUPT_STACK *stack)
{
	if (__inword(FIXED_ACPI_TABLE->PM1X.EVTA) & FADT_PM1E_ST_PWRBTN_STS)
	{
		__outword(FIXED_ACPI_TABLE->PM1X.EVTA, FADT_PM1E_ST_PWRBTN_STS);
		while (__inword(FIXED_ACPI_TABLE->PM1X.EVTA) & FADT_PM1E_ST_PWRBTN_STS) {}
		if (FIXED_ACPI_TABLE->PM1X.EVTB)
		{
			__outword(FIXED_ACPI_TABLE->PM1X.EVTB, FADT_PM1E_ST_PWRBTN_STS);
			while (__inword(FIXED_ACPI_TABLE->PM1X.EVTB) & FADT_PM1E_ST_PWRBTN_STS) {}
		}
	}
	eoi_apic(0);
	acpi_shutdown();
}
void setup_fadt(volatile ACPI_FADT *fadt)
{
	if (!FIXED_ACPI_TABLE)
	{
		FIXED_ACPI_TABLE = fadt;
		return;
	}

	setup_fadt_s5();

	if (ioapic_redirect(FIXED_ACPI_TABLE->SCIV, FIXED_ACPI_TABLE->SCIV + 0x20))
	{
		printf("I/O APIC Redirect IRQ FAILED\n");
		return;
	}
	register_interrupt(FIXED_ACPI_TABLE->SCIV + 0x20, system_control_interrupt);

	if (!FIXED_ACPI_TABLE->SMIC)
		return;
	if (!FIXED_ACPI_TABLE->ACPE)
		return;
	__outbyte(FIXED_ACPI_TABLE->SMIC, FIXED_ACPI_TABLE->ACPE);
	while (!(__inword(FIXED_ACPI_TABLE->PM1X.CNTA) & FADT_PM1E_ST_SCI_EN)) __halt();
	if (FIXED_ACPI_TABLE->PM1X.CNTB)
		while (!(__inword(FIXED_ACPI_TABLE->PM1X.CNTB) & FADT_PM1E_ST_SCI_EN)) __halt();

	DWORD len = FIXED_ACPI_TABLE->P1EL >> 1;
	DWORD enReg1a = FIXED_ACPI_TABLE->PM1X.EVTA + len;
	DWORD enReg1b = FIXED_ACPI_TABLE->PM1X.EVTB;
	if (enReg1b)
		enReg1b += len;

	__outword(enReg1a, FADT_PM1E_EN_PWRBTN_EN);
	if (enReg1b)
		__outword(enReg1b, FADT_PM1E_EN_PWRBTN_EN);
}
void acpi_shutdown()
{
	if (!(FADT_STATUS & FADT_STAT_S5))
		return;
	__outword(FIXED_ACPI_TABLE->PM1X.CNTA, FADT_PM1C_SLP_EN | PM1_SLP_TYPE5_A);
	if (FIXED_ACPI_TABLE->PM1X.CNTB)
		__outword(FIXED_ACPI_TABLE->PM1X.CNTB, FADT_PM1C_SLP_EN | PM1_SLP_TYPE5_B);
}