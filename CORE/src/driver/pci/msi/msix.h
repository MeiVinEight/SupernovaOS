//
// Created by MeiVi on 2026/03/27.
//

#ifndef SUPERNOVA_MSIX_H
#define SUPERNOVA_MSIX_H

#include <types.h>
#include <driver/pci/pcie.h>

typedef struct _PCI_EXPRESS_CAPABILITY_MSI_X
{
	/**
	 * Capability ID [RO] - Indicates the MSI-X Capability structure. This field must return a Capability ID of 11h
	 * indicating that this is an MSI-X Capability structure.
	 */
	BYTE  CAID;
	/**
	 * Next Capability Pointer [RO] - This field contains the offset to the next PCI Capability structure or 00h if no
	 * other items exist in the linked list of Capabilities.
	 */
	BYTE  NEXT;
	/**
	 * Table Size [RO] - System software reads this field to determine the MSI-X Table Size N, which is encoded as
	 * N-1. For example, a returned value of 000 0000 0011b indicates a table size of 4.
	 */
	WORD  TABS:11;
	/**
	 * Reserved Reserved - Always returns 0 on a read, and a write operation has no effect.
	 */
	WORD  RSV0:3;
	/**
	 * Function Mask [RW] - If Set, all of the vectors associated with the Function are masked, regardless of their
	 * per-vector Mask bit values.
	 *
	 * If Clear, each vector’s Mask bit determines whether the vector is masked or not.
	 *
	 * Setting or Clearing the MSI-X Function Mask bit has no effect on the value of the per-vector Mask bits.
	 *
	 * Default value of this bit is 0b.
	 */
	WORD  FMSK:1;
	/**
	 * MSI-X Enable [RW] - If Set and the MSI Enable bit in the Message Control Register for MSI (see § Section 7.7.1.2
	 * ) is Clear, the Function is permitted to use MSI-X to request service and is prohibited from using INTx
	 * interrupts (if implemented). System configuration software Sets this bit to enable MSI-X.
	 *
	 * If Clear, the Function is prohibited from using MSI-X to request service.
	 *
	 * Software changing this bit during active operation may result in the Function dropping pending
	 * interrupt conditions or failing to recognize new interrupt conditions. See § Section 6.1.4.5 .
	 * Default value of this bit is 0b.
	 */
	WORD  MXEN:1;
	/**
	 * Table BIR [RO] - Indicates which one of a Function’s Base Address Registers, located beginning at 10h in
	 * Configuration Space, or entry in the Enhanced Allocation capability with a matching BAR Equivalent
	 * Indicator (BEI), is used to map the Function’s MSI-X Table into Memory Space.
	 *
	 * Defined encodings are:
	 * - 0 Base Address Register 10h
	 * - 1 Base Address Register 14h
	 * - 2 Base Address Register 18h
	 * - 3 Base Address Register 1Ch
	 * - 4 Base Address Register 20h
	 * - 5 Base Address Register 24h
	 * - 6 Reserved
	 * - 7 Reserved
	 *
	 * For a 64-bit Base Address Register, the Table BIR indicates the lower DWORD. For Functions with Type 1
	 * Configuration Space headers, BIR values 2 through 5 are also Reserved.
	 */
	DWORD TBIR:3;
	/**
	 * Table Offset [RO] - Used as an offset from the address contained by one of the Function’s Base Address
	 * Registers to point to the base of the MSI-X Table. The lower 3 Table BIR bits are masked off (set to zero) by
	 * software to form a 32-bit QWORD-aligned offset.
	 *
	 * For VFs, the Table Offset value is relative to the VF’s Memory address space.
	 */
	DWORD TOFF:29;
	/**
	 * PBA BIR [RO] - Indicates which one of a Function’s Base Address Registers, located beginning at 10h in
	 * Configuration Space, or entry in the Enhanced Allocation capability with a matching BEI, is used to map
	 * the Function’s MSI-X PBA into Memory Space.
	 *
	 * The PBA BIR value definitions are identical to those for the Table BIR.
	 */
	DWORD PBIR:3;
	/**
	 * PBA Offset [RO] - Used as an offset from the address contained by one of the Function’s Base Address
	 * Registers to point to the base of the MSI-X PBA. The lower 3 PBA BIR bits are masked off (set to zero) by
	 * software to form a 32-bit QWORD-aligned offset.
	 *
	 * For VFs, the PBA Offset value is relative to the VF’s Memory address space.
	 */
	DWORD POFF:29;
} PCI_EXPRESS_CAPABILITY_MSI_X;
typedef struct _PCI_EXPRESS_MSIX_TABLE_ENTRY
{
	QWORD ADDR;
	DWORD DATA;
	DWORD CTRL;
} PCI_EXPRESS_MSIX_TABLE_ENTRY;

DWORD pcie_setup_msix(PCI_EXPRESS_DEVICE *device, DWORD intx);
void clear_msix_pending_bit(DWORD *pba, DWORD entryIdx);

#endif //SUPERNOVA_MSIX_H
