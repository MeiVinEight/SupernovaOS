#include <timer/hpet.h>
#include <core.h>
#include <intrinsic.h>

COREAPI ACPI_HPET HPET_TABLE;
COREAPI HPET_REGISTER *volatile HPET_REGISTERS = 0;

void setup_hpet(ACPI_HPET *hpet)
{
	HPET_TABLE = *hpet;

	QWORD hpetAddr = core_mapping(HPET_TABLE.ADDR.ADDR);
	HPET_REGISTERS = (HPET_REGISTER *) hpetAddr;

	// Flush TLB
	__writecr3(__readcr3());

	// Enable th HPET by setting the ENABLE bit in the General Configuration Register
	QWORD gcfr = HPET_REGISTERS->GCFR;
	gcfr |= HPET_GCFR_ENABLE;
	HPET_REGISTERS->GCFR = gcfr;
}
QWORD hpet_query_frequency()
{
	if (!HPET_REGISTERS)
		return 0;
	QWORD gcir = HPET_REGISTERS->GCIR;
	DWORD period = gcir >> 32;
	// Convert the period from femproseconds to HZ
	if (period)
		period = (1000000000000000ULL) / period;
	return period;
}
QWORD hpet_get_counter()
{
	return HPET_REGISTERS->MCVR;
}