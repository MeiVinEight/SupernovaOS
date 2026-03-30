//
// Created by MeiVi on 03/29/2026.
//

#ifndef SUPERNOVA_FADT_H
#define SUPERNOVA_FADT_H

#include <types.h>
#include <acpi/acpi.h>

#define FADT_PM1E_ST_SCI_EN     (1 << 0)
#define FADT_PM1E_ST_PWRBTN_STS (1 << 8)
#define FADT_PM1E_EN_PWRBTN_EN  (1 << 8)
#define FADT_PM1C_SLP_EN        (1 << 13)

#define FADT_STAT_S5 (1ULL << 0)

extern volatile QWORD FADT_STATUS;
extern volatile DWORD PM1_SLP_TYPE5_A;
extern volatile DWORD PM1_SLP_TYPE5_B;

typedef struct _ACPI_FADT_PM1
{
	/**
	 * System port address of the PM1a Event Register Block.
	 * See Section 4.8.3.1 for a hardware description layout
	 * of this register block. This is a required field. If the
	 * X_PM1a_CNT_BLK field contains a non zero value
	 * which can be used by the OSPM, then this field must
	 * be ignored by the OSPM.
	 */
	DWORD EVTA;
	/**
	 * System port address of the PM1b Event Register Block.
	 * See Section 4.8.3.1 for a hardware description layout of
	 * this register block. This field is optional; if this register block is not supported, this field contains zero. If
	 * the X_PM1b_EVT_BLK field contains a non zero value
	 * which can be used by the OSPM, then this field must be
	 * ignored by the OSPM.
	 */
	DWORD EVTB;
	/**
	 * System port address of the PM1a Control Register
	 * Block. See Section 4.8.3.1 for a hardware description
	 * layout of this register block. This is a required field. If
	 * the X_PM1a_CNT_BLK field contains a non zero value
	 * which can be used by the OSPM, then this field must be
	 * ignored by the OSPM.
	 */
	DWORD CNTA;
	/**
	 * System port address of the PM1b Control Register
	 * Block. See Section 4.8.3.1 for a hardware description
	 * layout of this register block. This field is optional; if this
	 * register block is not supported, this field contains zero. If
	 * the X_PM1b_CNT_BLK field contains a non zero value
	 * which can be used by the OSPM, then this field must be
	 * ignored by the OSPM.
	 */
	DWORD CNTB;
} ACPI_FADT_PM1;
#pragma pack(push, 4)
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
	ACPI_FADT_PM1 PM1X;
	/**
	 * System port address of the PM2 Control Register Block.
	 * See Table 4.4 for a hardware description layout of this
	 * register block. This field is optional; if this register block is not supported, this field contains zero. If
	 * the X_PM2_CNT_BLK field contains a non zero value
	 * which can be used by the OSPM, then this field must be
	 * ignored by the OSPM.
	 */
	DWORD PM2C;
	/**
	 * System port address of the Power Management Timer
	 * Control Register Block. See the Section 4.8.3.3 for a
	 * hardware description layout of this register block. This
	 * is an optional field; if this register block is not supported,
	 * this field contains zero. If the X_PM_TMR_BLK field
	 * contains a non-zero value which can be used by the
	 * OSPM, then this field must be ignored by the OSPM.
	 */
	DWORD PMTB;
	/**
	 * System port address of General-Purpose Event 0 Register Block. See Section 4.8.4.1 for more information.
	 * If this register block is not supported, this field contains zero. If the X_GPE0_BLK field contains a nonzero
	 * value which can be used by the OSPM, then this field
	 * must be ignored by the OSPM.
	 */
	DWORD GPE0;
	/**
	 * System port address of General-Purpose Event 1 Register Block. See Section 4.8.4.1 for more information.
	 * This is an optional field; if this register block is not supported, this field contains zero. If the X_GPE1_BLK
	 * field contains a nonzero value which can be used by the
	 * OSPM, then this field must be ignored by the OSPM.
	 */
	DWORD GPE1;
	/**
	 * Number of bytes decoded by PM1a_EVT_BLK and, if
	 * supported, PM1b_ EVT_BLK. This value is >= 4.
	 */
	BYTE  P1EL;
	/**
	 * Number of bytes decoded by PM1a_CNT_BLK and, if
	 * supported, PM1b_CNT_BLK. This value is >= 2.
	 */
	BYTE  P1CL;
	/**
	 * Number of bytes decoded by PM2_CNT_BLK. Support
	 * for the PM2 register block is optional. If supported, this
	 * value is >= 1. If not supported, this field contains zero.
	 */
	BYTE  P2CL;
	/**
	 * Number of bytes decoded by PM_TMR_BLK. If the PM
	 * Timer is supported, this field’s value must be 4. If not
	 * supported, this field contains zero.
	 */
	BYTE  PMTL;
	/**
	 * The length of the register whose address is given by
	 * X_GPE0_BLK (if nonzero) or by GPE0_BLK (otherwise) in bytes. The value is a non-negative multiple of
	 * 2.
	 */
	BYTE  G0BL;
	/**
	 * The length of the register whose address is given by
	 * X_GPE1_BLK (if nonzero) or by GPE1_BLK (otherwise) in bytes. The value is a non-negative multiple of
	 * 2.
	 */
	BYTE  G1BL;
	/**
	 * Offset within the ACPI general-purpose event model
	 * where GPE1 based events start.
	 */
	BYTE  GP1A;
	/**
	 * If non-zero, this field contains the value OSPM writes
	 * to the SMI_CMD register to indicate OS support for the
	 * _CST object and C States Changed notification.
	 */
	BYTE  CSTC;
	DWORD RSV1[11];
	/**
	 * Extended physical address of the DSDT. If this field contains a nonzero value which can be used by the OSPM,
	 * then the DSDT field must be ignored by the OSPM.
	 */
	QWORD XDSD;
} ACPI_FADT;
#pragma pack(pop)

void setup_fadt(volatile ACPI_FADT *FADT);
void acpi_shutdown();

#endif //SUPERNOVA_FADT_H
