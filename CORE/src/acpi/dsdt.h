//
// Created by MeiVi on 2026/03/30.
//

#ifndef SUPERNOVA_DSDT_H
#define SUPERNOVA_DSDT_H

#include <acpi/acpi.h>

#define DSDT_SIGNATURE_S5 0x5F35535F

typedef struct _ACPI_DSDT
{
	ACPI_SDT_HEADER HEAD;
	BYTE DATA[];
} ACPI_DSDT;

#endif //SUPERNOVA_DSDT_H
