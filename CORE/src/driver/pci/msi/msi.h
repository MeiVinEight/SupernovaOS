//
// Created by MeiVi on 2026/03/27.
//

#ifndef SUPERNOVA_MSI_H
#define SUPERNOVA_MSI_H

#define MSI_ADDRESS_BASE (0xFEE00000)

#include <types.h>
#include <driver/pci/pcie.h>

typedef struct _PCI_EXPRESS_CAPABILITY_MSI32
{
	/**
	 * Capability ID - Indicates the MSI Capability structure. This field must return a Capability ID of 05h
	 * indicating that this is an MSI Capability structure.
	 */
	BYTE CAID;
	/**
	 * Next Capability Pointer - This field contains the offset to the next PCI Capability structure or 00h if no
	 * other items exist in the linked list of Capabilities.
	 */
	BYTE NEXT;
	/**
	 * MSI Enable - [RW] If Set and the MSI-X Enable bit in the Message Control Register for MSI-X (see § Section
	 * 7.7.2.2 ) is Clear, the Function is permitted to use MSI to request service and is prohibited from using
	 * INTx interrupts. System configuration software Sets this bit to enable MSI. Refer to § Section 7.5.1.1.3 for
	 * control of INTx interrupts.
	 *
	 * If Clear, the Function is prohibited from using MSI to request service.
	 *
	 * Software changing this bit during active operation may result in the Function dropping pending
	 * interrupt conditions or failing to recognize new interrupt conditions. See § Section 6.1.4.5 .
	 *
	 * Default value of this bit is 0b.
	 */
	WORD MSIE:1;
	/**
	 * Multiple Message Capable - [RO] System software reads this field to determine the number of requested
	 * vectors. The number of requested vectors must be aligned to a power of two (if a Function requires three
	 * vectors, it requests four by initializing this field to 010b). The encoding is defined as:
	 * - 000b 1 vector requested
	 * - 001b 2 vectors requested
	 * - 010b 4 vectors requested
	 * - 011b 8 vectors requested
	 * - 100b 16 vectors requested
	 * - 101b 32 vectors requested
	 * - 110b Reserved
	 * - 111b Reserved
	 */
	WORD MMCA:3;
	/**
	 * Multiple Message Enable - [RW] software writes to this field to indicate the number of allocated vectors. The
	 * number of allocated vectors is aligned to a power of two. As an example, if a Function requests four
	 * vectors (indicated by a Multiple Message Capable encoding of 010b), software can allocate either four, two,
	 * or one vector by writing a 010b, 001b, or 000b to this field, respectively.
	 *
	 * Behavior is undefined if the number of vectors allocated is greater than the number of vectors
	 * requested.
	 *
	 * Behavior is undefined if this field is changed while MSI Enable is Set.
	 *
	 * When MSI Enable is Set, a Function will be allocated at least 1 vector. The encoding is defined as:
	 * - 000b 1 vector allocated
	 * - 001b 2 vectors allocated
	 * - 010b 4 vectors allocated
	 * - 011b 8 vectors allocated
	 * - 100b 16 vectors allocated
	 * - 101b 32 vectors allocated
	 * - 110b Reserved
	 * - 111b Reserved
	 *
	 * Function behavior is undefined if software changes the value of this field while the MSI Enable bit is Set.
	 * Default value of this field is 000b.
	 */
	WORD MMEN:3;
	/**
	 * 64-bit Address Capable - [RO] If Set, the Function is capable of sending a 64-bit Message Address. If Clear, the
	 * Function is not capable of sending a 64-bit Message Address. This bit must be Set if the Function is a PCI
	 * Express Endpoint, as indicated by the value in the Device/Port Type field. This bit MUST@FLIT be Set.
	 */
	WORD AC64:1;
	/**
	 * Per-Vector Masking Capable - [RO] If Set, the Function supports MSI Per-Vector Masking. If Clear, the
	 * Function does not support MSI Per-Vector Masking. This bit must be Set if the Function an SR-IOV PF or
	 * an SR-IOV VF. This bit is permitted to be Set or Clear if the Function is an SIOV PF.
	 */
	WORD PVMC:1;
	/**
	 * Extended Message Data Capable - [RO] If Set, the Function is capable of providing Extended Message Data.
	 * If Clear, the Function does not support providing Extended Message Data.
	 */
	WORD EMDC:1;
	/**
	 * Extended Message Data Enable - [RW/RO] If Set, the Function is enabled to provide Extended Message Data.
	 * If Clear, the Function is not enabled to provide Extended Message Data.
	 *
	 * Default value of this bit is 0b.
	 *
	 * This bit must be read-write if the Extended Message Data Capable bit is 1b; otherwise it must be hardwired
	 * to 0b.
	 */
	WORD EMDE:1;
	WORD RSV0:5;
	/**
	 * Message Address - System-specified message address.
	 *
	 * If the MSI Enable bit is Set, the contents of this register specify the DWORD-aligned address
	 * (Address[31:02]) for the MSI transaction. Address[1:0] are set to 00b.
	 *
	 * Default value of this field is undefined.
	 */
	DWORD ADDR;
	/**
	 * Message Data - System-specified message data.
	 *
	 * If the MSI Enable bit is Set, the Function sends a DWORD Memory Write transaction using Message Data
	 * for the lower 16 bits. All 4 Byte Enables are Set.
	 *
	 * The Multiple Message Enable field defines the number of low order message data bits the Function is
	 * permitted to modify to generate its system software allocated vectors. For example, a Multiple Message
	 * Enable encoding of 010b indicates the Function has been allocated four vectors and is permitted to
	 * modify message data bits 1 and 0 (a Function modifies the lower message data bits to generate the
	 * allocated number of vectors). If the Multiple Message Enable field is 000b, the Function is not permitted to
	 * modify the message data. When Multiple Message Enable is non-zero, behavior is undefined if the
	 * corresponding low order bits of this register are not 0b.
	 *
	 * Default value of this field is undefined.
	 */
	WORD DATA;
	/**
	 * Extended Message Data - System-specified message data.
	 *
	 * This register is optional. For the MSI Capability structures without Per-vector Masking, it must be
	 * implemented if the Extended Message Data Capable bit is Set; otherwise, it is outside the MSI
	 * Capability structure and undefined. For the MSI Capability structures with Per-vector Masking, it
	 * must be implemented if the Extended Message Data Capable bit is Set; otherwise, it is RsvdP.
	 *
	 * If the Extended Message Data Enable bit is Set, the DWORD Memory Write transaction uses Extended
	 * Message Data for the upper 16 bits; otherwise, it uses 0000h for the upper 16 bits.
	 *
	 * Default value of this field is 0000h.
	 */
	WORD DATX;
	/**
	 * Mask Bits - For each Mask bit that is Set, the Function is prohibited from sending the associated
	 * message.
	 *
	 * Default is 0.
	 */
	DWORD MASK;
	/**
	 * Pending Bits - For each Pending bit that is Set, the Function has a pending associated message.
	 *
	 * Default is 0.
	 */
	DWORD PEND;
} PCI_EXPRESS_CAPABILITY_MSI32;
#pragma pack(push, 4)
typedef struct _PCI_EXPRESS_CAPABILITY_MSI64
{
	/**
	 * Capability ID - Indicates the MSI Capability structure. This field must return a Capability ID of 05h
	 * indicating that this is an MSI Capability structure.
	 */
	BYTE CAID;
	/**
	 * Next Capability Pointer - This field contains the offset to the next PCI Capability structure or 00h if no
	 * other items exist in the linked list of Capabilities.
	 */
	BYTE NEXT;
	/**
	 * MSI Enable - [RW] If Set and the MSI-X Enable bit in the Message Control Register for MSI-X (see § Section
	 * 7.7.2.2 ) is Clear, the Function is permitted to use MSI to request service and is prohibited from using
	 * INTx interrupts. System configuration software Sets this bit to enable MSI. Refer to § Section 7.5.1.1.3 for
	 * control of INTx interrupts.
	 *
	 * If Clear, the Function is prohibited from using MSI to request service.
	 *
	 * Software changing this bit during active operation may result in the Function dropping pending
	 * interrupt conditions or failing to recognize new interrupt conditions. See § Section 6.1.4.5 .
	 *
	 * Default value of this bit is 0b.
	 */
	WORD MSIE:1;
	/**
	 * Multiple Message Capable - [RO] System software reads this field to determine the number of requested
	 * vectors. The number of requested vectors must be aligned to a power of two (if a Function requires three
	 * vectors, it requests four by initializing this field to 010b). The encoding is defined as:
	 * - 000b 1 vector requested
	 * - 001b 2 vectors requested
	 * - 010b 4 vectors requested
	 * - 011b 8 vectors requested
	 * - 100b 16 vectors requested
	 * - 101b 32 vectors requested
	 * - 110b Reserved
	 * - 111b Reserved
	 */
	WORD MMCA:3;
	/**
	 * Multiple Message Enable - [RW] software writes to this field to indicate the number of allocated vectors. The
	 * number of allocated vectors is aligned to a power of two. As an example, if a Function requests four
	 * vectors (indicated by a Multiple Message Capable encoding of 010b), software can allocate either four, two,
	 * or one vector by writing a 010b, 001b, or 000b to this field, respectively.
	 *
	 * Behavior is undefined if the number of vectors allocated is greater than the number of vectors
	 * requested.
	 *
	 * Behavior is undefined if this field is changed while MSI Enable is Set.
	 *
	 * When MSI Enable is Set, a Function will be allocated at least 1 vector. The encoding is defined as:
	 * - 000b 1 vector allocated
	 * - 001b 2 vectors allocated
	 * - 010b 4 vectors allocated
	 * - 011b 8 vectors allocated
	 * - 100b 16 vectors allocated
	 * - 101b 32 vectors allocated
	 * - 110b Reserved
	 * - 111b Reserved
	 *
	 * Function behavior is undefined if software changes the value of this field while the MSI Enable bit is Set.
	 * Default value of this field is 000b.
	 */
	WORD MMEN:3;
	/**
	 * 64-bit Address Capable - [RO] If Set, the Function is capable of sending a 64-bit Message Address. If Clear, the
	 * Function is not capable of sending a 64-bit Message Address. This bit must be Set if the Function is a PCI
	 * Express Endpoint, as indicated by the value in the Device/Port Type field. This bit MUST@FLIT be Set.
	 */
	WORD AC64:1;
	/**
	 * Per-Vector Masking Capable - [RO] If Set, the Function supports MSI Per-Vector Masking. If Clear, the
	 * Function does not support MSI Per-Vector Masking. This bit must be Set if the Function an SR-IOV PF or
	 * an SR-IOV VF. This bit is permitted to be Set or Clear if the Function is an SIOV PF.
	 */
	WORD PVMC:1;
	/**
	 * Extended Message Data Capable - [RO] If Set, the Function is capable of providing Extended Message Data.
	 * If Clear, the Function does not support providing Extended Message Data.
	 */
	WORD EMDC:1;
	/**
	 * Extended Message Data Enable - [RW/RO] If Set, the Function is enabled to provide Extended Message Data.
	 * If Clear, the Function is not enabled to provide Extended Message Data.
	 *
	 * Default value of this bit is 0b.
	 *
	 * This bit must be read-write if the Extended Message Data Capable bit is 1b; otherwise it must be hardwired
	 * to 0b.
	 */
	WORD EMDE:1;
	WORD RSV0:5;
	/**
	 * Message Address - System-specified message address.
	 *
	 * If the MSI Enable bit is Set, the contents of this register specify the DWORD-aligned address
	 * (Address[31:02]) for the MSI transaction. Address[1:0] are set to 00b.
	 *
	 * Default value of this field is undefined.
	 */
	QWORD ADDR;
	/**
	 * Message Data - System-specified message data.
	 *
	 * If the MSI Enable bit is Set, the Function sends a DWORD Memory Write transaction using Message Data
	 * for the lower 16 bits. All 4 Byte Enables are Set.
	 *
	 * The Multiple Message Enable field defines the number of low order message data bits the Function is
	 * permitted to modify to generate its system software allocated vectors. For example, a Multiple Message
	 * Enable encoding of 010b indicates the Function has been allocated four vectors and is permitted to
	 * modify message data bits 1 and 0 (a Function modifies the lower message data bits to generate the
	 * allocated number of vectors). If the Multiple Message Enable field is 000b, the Function is not permitted to
	 * modify the message data. When Multiple Message Enable is non-zero, behavior is undefined if the
	 * corresponding low order bits of this register are not 0b.
	 *
	 * Default value of this field is undefined.
	 */
	WORD DATA;
	/**
	 * Extended Message Data - System-specified message data.
	 *
	 * This register is optional. For the MSI Capability structures without Per-vector Masking, it must be
	 * implemented if the Extended Message Data Capable bit is Set; otherwise, it is outside the MSI
	 * Capability structure and undefined. For the MSI Capability structures with Per-vector Masking, it
	 * must be implemented if the Extended Message Data Capable bit is Set; otherwise, it is RsvdP.
	 *
	 * If the Extended Message Data Enable bit is Set, the DWORD Memory Write transaction uses Extended
	 * Message Data for the upper 16 bits; otherwise, it uses 0000h for the upper 16 bits.
	 *
	 * Default value of this field is 0000h.
	 */
	WORD DATX;
	/**
	 * Mask Bits - For each Mask bit that is Set, the Function is prohibited from sending the associated
	 * message.
	 *
	 * Default is 0.
	 */
	DWORD MASK;
	/**
	 * Pending Bits - For each Pending bit that is Set, the Function has a pending associated message.
	 *
	 * Default is 0.
	 */
	DWORD PEND;
} PCI_EXPRESS_CAPABILITY_MSI64;
#pragma pack(pop)

DWORD pcie_setup_msi(volatile PCI_EXPRESS_DEVICE *device, DWORD intx);

#endif //SUPERNOVA_MSI_H
