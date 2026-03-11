#include <acpi/acpi.h>

#include "intrinsic.h"

typedef struct _ACPI_RSDP
{
	char SIGN[8];
	BYTE CHKS;
	char OEMS[6];
	BYTE REVI;
	DWORD RSDT;
	// ACPI 2.0
	DWORD LENH;
	QWORD XSDT;
	BYTE XCHK;
	BYTE RSV0[3];
} ACPI_RSDP;
