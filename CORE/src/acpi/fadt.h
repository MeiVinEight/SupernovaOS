//
// Created by MeiVi on 03/29/2026.
//

#ifndef SUPERNOVA_FADT_H
#define SUPERNOVA_FADT_H

#include <types.h>
#include <acpi/acpi.h>

typedef struct _ACPI_FADT
{
	ACPI_SDT_HEADER HEAD;
	/**
	 * Physical memory address of the FACS, where OSPM
	 * and Firmware exchange control information. See Section 5.2.10 for more information about the FACS. If
	 * the X_FIRMWARE_CTRL field contains a non zero
	 * value which can be used by the OSPM, then this
	 * field must be ignored by the OSPM. If the HARDWARE_REDUCED_ACPI flag is set, and both this field
	 * and the X_FIRMWARE_CTRL field are zero, there is
	 * no FACS available.
	 */
	DWORD CTRL;
	/**
	 * Physical memory address of the DSDT. If the X_DSDT
	 * field contains a non-zero value which can be used by the
	 * OSPM, then this field must be ignored by the OSPM.
	 */
	DWORD DSDT;
	/**
	 * ACPI 1.0 defined this offset as a field named
	 * INT_MODEL, which was eliminated in ACPI 2.0.
	 * Platforms should set this field to zero but field values
	 * of one are also allowed to maintain compatibility with
	 * ACPI 1.0.
	 */
	BYTE  RSV0;
	/**
	 * This field is set by the OEM to convey the preferred
	 * power management profile to OSPM. OSPM can use
	 * this field to set default power management policy
	 * parameters during OS installation. Field Values:
	 * - 0 Unspecified
	 * - 1 Desktop
	 * - 2 Mobile
	 * - 3 Workstation
	 * - 4 Enterprise Server
	 * - 5 SOHO Server
	 * - 6 Appliance PC
	 * - 7 Performance Server
	 * - 8 Tablet
	 * - >8 Reserved
	 */
	BYTE  PPMP;
	/**
	 * System vector the SCI interrupt is wired to in 8259 mode.
	 *
	 * On systems that do not contain the 8259, this field contains
	 * the Global System Interrupt number of the SCI interrupt.
	 *
	 * OSPM is required to treat the ACPI SCI interrupt as a shareable,
	 * level, active low interrupt.
	 */
	WORD  SCIV;
	/**
	 * System port address of the SMI Command Port. During ACPI OS initialization, OSPM can determine that
	 * the ACPI hardware registers are owned by SMI (by way
	 * of the SCI_EN bit), in which case the ACPI OS issues
	 * the ACPI_ENABLE command to the SMI_CMD port.
	 * The SCI_EN bit effectively tracks the ownership of the
	 * ACPI hardware registers. OSPM issues commands to
	 * the SMI_CMD port synchronously from the boot processor. This field is reserved and must be zero on system
	 * that does not support System Management mode.
	 */
	DWORD SMIC;
	/**
	 * The value to write to SMI_CMD to disable SMI ownership of the ACPI hardware registers. The last action SMI
	 * does to relinquish ownership is to set the SCI_EN bit.
	 * During the OS initialization process, OSPM will synchronously wait for the ntransfer of SMI ownership to
	 * complete, so the ACPI system releases SMI ownership
	 * as quickly as possible. This field is reserved and must be
	 * zero on systems that do not support Legacy Mode.
	 */
	BYTE  ACPE;
	/**
	 * The value to write to SMI_CMD to re-enable SMI ownership of the ACPI hardware registers. This can only
	 * be done when ownership was originally acquired from
	 * SMI by OSPM using ACPI_ENABLE. An OS can hand
	 * ownership back to SMI by relinquishing use to the ACPI
	 * hardware registers, masking off all SCI interrupts, clearing the SCI_EN bit and then writing ACPI_DISABLE to
	 * the SMI_CMD port from the boot processor. This field
	 * is reserved and must be zero on systems that do not support Legacy Mode.
	 */
	BYTE  ACPD;
	/**
	 * The value to write to SMI_CMD to enter the S4BIOS
	 * state. The S4BIOS state provides an alternate way to enter the S4 state where the firmware saves and restores the
	 * memory context. A value of zero in S4BIOS_F indicates
	 * S4BIOS_REQ is not supported. (See Section 5.2.10)
	 */
	BYTE  S4BR;
	/**
	 * If non-zero, this field contains the value OSPM writes
	 * to the SMI_CMD register to assume processor performance state control responsibility.
	 */
	BYTE  PSCN;
} ACPI_FADT;

#endif //SUPERNOVA_FADT_H
