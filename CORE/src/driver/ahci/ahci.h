#pragma once

#include <types.h>
#include <driver/pci/pcie.h>
#include <async/async.h>

#define HBA_PORT_CMD_ST      (1 <<  0)
#define HBA_PORT_CMD_SPIN_UP (1 <<  1)
#define HBA_PORT_CMD_CLO     (1 <<  3)
#define HBA_PORT_CMD_FRE     (1 <<  4)
#define HBA_PORT_CMD_FR      (1 << 14)
#define HBA_PORT_CMD_CR      (1 << 15)

#define HBA_PORT_IPM_ACTIVE  1
#define HBA_PORT_DET_PRESENT 3

#define	HBA_PORT_SIG_ATA    0x00000101  // SATA drive
#define	HBA_PORT_SIG_ATAPI  0xEB140101  // SATAPI drive
#define	HBA_PORT_SIG_SEMB   0xC33C0101  // Enclosure management bridge
#define	HBA_PORT_SIG_PM     0x96690101  // Port multiplier

#define HBA_PORT_IS_TFES   (1 << 30)       /* TFES - Task File Error Status */

typedef struct _AHCI_CONTROLLER_PORT
{
	/**
	 * Command List Base Address (CLB) - RW: Indicates the 32-bit base physical address for
	 * the command list for this port. This base is used when fetching commands to execute.
	 * The structure pointed to by this address range is 1K-bytes in length. This address must
	 * be 1K-byte aligned as indicated by bits 09:00 being read only.
	 */
	QWORD CMDA;
	/**
	 * FIS Base Address (FB) - RW: Indicates the 32-bit base physical address for received
	 * FISes. The structure pointed to by this address range is 256 bytes in length. This
	 * address must be 256-byte aligned as indicated by bits 07:00 being read only. When
	 * FIS-based switching is in use, this structure is 4KB in length and the address shall be
	 * 4KB aligned (refer to section 9.3.3).
	 */
	QWORD FISA;
	DWORD INTS;
	DWORD INTE;
	DWORD PCMD;
	DWORD RSV0;
	DWORD PTFD;
	DWORD SIGN;
	DWORD STAT;
	DWORD CTRL;
	DWORD ERRO;
	DWORD ACTV;
	DWORD CISS;
	DWORD SNTF;
	DWORD FBSC;
	DWORD SLEP;
	DWORD RSV1[14];
} AHCI_CONTROLLER_PORT;
typedef struct _AHCI_CONTROLLER_GENERAL
{
	/**
	 * Number of Ports (NP): 0’s based value indicating the maximum number of ports
	 * supported by the HBA silicon. A maximum of 32 ports can be supported. A value of
	 * ‘0h’, indicating one port, is the minimum requirement. Note that the number of ports
	 * indicated in this field may be more than the number of ports indicated in the PI register.
	 */
	DWORD PCNT:5;
	/**
	 * Supports External SATA (SXS): When set to ‘1’, indicates that the HBA has one or
	 * more Serial ATA ports that has a signal only connector that is externally accessible
	 * (e.g. eSATA connector). If this bit is set to ‘1’, software may refer to the PxCMD.ESP bit
	 * to determine whether a specific port has its signal connector externally accessible as a
	 * signal only connector (i.e. power is not part of that connector). When the bit is cleared
	 * to ‘0’, indicates that the HBA has no Serial ATA ports that have a signal only connector
	 * externally accessible.
	 */
	DWORD SEXS:1;
	/**
	 * Enclosure Management Supported (EMS): When set to ‘1’, indicates that the HBA
	 * supports enclosure management as defined in section 12. When enclosure
	 * management is supported, the HBA has implemented the EM_LOC and EM_CTL
	 * global HBA registers. When cleared to ‘0’, indicates that the HBA does not support
	 * enclosure management and the EM_LOC and EM_CTL global HBA registers are not
	 * implemented.
	 */
	DWORD EMSS:1;
	/**
	 * Command Completion Coalescing Supported (CCCS): When set to ‘1’, indicates
	 * that the HBA supports command completion coalescing as defined in section 11. When
	 * command completion coalescing is supported, the HBA has implemented the
	 * CCC_CTL and the CCC_PORTS global HBA registers. When cleared to ‘0’, indicates
	 * that the HBA does not support command completion coalescing and the CCC_CTL and
	 * CCC_PORTS global HBA registers are not implemented.
	 */
	DWORD CCCS:1;
	/**
	 * Number of Command Slots (NCS) - RO: 0’s based value indicating the number of
	 * command slots per port supported by this HBA. A minimum of 1 and maximum of 32
	 * slots per port can be supported. The same number of command slots is available on
	 * each implemented port.
	 */
	DWORD CSCN:5;
	/**
	 * Partial State Capable (PSC) - RO: Indicates whether the HBA can support transitions to the
	 * Partial state. When cleared to ‘0’, software must not allow the HBA to initiate transitions
	 * to the Partial state via agressive link power management nor the PxCMD.ICC field in
	 * each port, and the PxSCTL.IPM field in each port must be programmed to disallow
	 * device initiated Partial requests. When set to ‘1’, HBA and device initiated Partial
	 * requests can be supported.
	 */
	DWORD PSCA:1;
	/**
	 * Slumber State Capable (SSC) - RO: Indicates whether the HBA can support transitions to
	 * the Slumber state. When cleared to ‘0’, software must not allow the HBA to initiate
	 * transitions to the Slumber state via agressive link power management nor the
	 * PxCMD.ICC field in each port, and the PxSCTL.IPM field in each port must be
	 * programmed to disallow device initiated Slumber requests. When set to ‘1’, HBA and
	 * device initiated Slumber requests can be supported.
	 */
	DWORD SSCA:1;
	/**
	 * PIO Multiple DRQ Block (PMD) - RO: If set to ‘1’, the HBA supports multiple DRQ block
	 * data transfers for the PIO command protocol. If cleared to ‘0’ the HBA only supports
	 * single DRQ block data transfers for the PIO command protocol. AHCI 1.2 HBAs shall
	 * have this bit set to ‘1’.
	 */
	DWORD PMDB:1;
	/**
	 * FIS-based Switching Supported (FBSS) - RO: When set to ‘1’, indicates that the HBA
	 * supports Port Multiplier FIS-based switching. When cleared to ‘0’, indicates that the
	 * HBA does not support FIS-based switching. This bit shall only be set to ‘1’ if the SPM
	 * bit is set to ‘1’.
	 */
	DWORD FBSS:1;
	/**
	 * Supports Port Multiplier (SPM) - RO: Indicates whether the HBA can support a Port
	 * Multiplier. When set, a Port Multiplier using command-based switching is supported
	 * and FIS-based switching may be supported. When cleared to ‘0’, a Port Multiplier is not
	 * supported, and a Port Multiplier may not be attached to this HBA.
	 */
	DWORD SPMU:1;
	/**
	 * Supports AHCI mode only (SAM) - RO: The SATA controller may optionally support AHCI
	 * access mechanisms only. A value of '0' indicates that in addition to the native AHCI
	 * mechanism (via ABAR), the SATA controller implements a legacy, task-file based
	 * register interface such as SFF-8038i. A value of '1' indicates that the SATA controller
	 * does not implement a legacy, task-file based register interface.
	 */
	DWORD SAMO:1;
	DWORD RSV0:1;
	/**
	 * Interface Speed Support (ISS) - RO: Indicates the maximum speed the HBA can support
	 * on its ports. These encodings match the system software programmable
	 *
	 * PxSCTL.DET.SPD field. Values are:
	 *  - Bits Definition
	 *  - 0000 Reserved
	 *  - 0001 Gen 1 (1.5 Gbps)
	 *  - 0010 Gen 2 (3 Gbps)
	 *  - 0011 Gen 3 (6 Gbps)
	 *  - 0100 - 1111 Reserved
	 */
	DWORD INSS:4;
	/**
	 * Supports Command List Override (SCLO) - RO: When set to ‘1’, the HBA supports the
	 * PxCMD.CLO bit and its associated function. When cleared to ‘0’, the HBA is not
	 * capable of clearing the BSY and DRQ bits in the Status register in order to issue a
	 * software reset if these bits are still set from a previous operation.
	 */
	DWORD SCLO:1;
	/**
	 * Supports Activity LED (SAL) - RO: When set to ‘1’, the HBA supports a single activity
	 * indication output pin. This pin can be connected to an LED on the platform to indicate
	 * device activity on any drive. When cleared to ‘0’, this function is not supported. See
	 * section 10.11 for more information.
	 */
	DWORD SLED:1;
	/**
	 * Supports Aggressive Link Power Management (SALP) - RO: When set to ‘1’, the HBA
	 * can support auto-generating link requests to the Partial or Slumber states when there
	 * are no commands to process. When cleared to ‘0’, this function is not supported and
	 * software shall treat the PxCMD.ALPE and PxCMD.ASP bits as reserved. Refer to
	 * section 8.3.1.3.
	 */
	DWORD SALP:1;
	/**
	 * Supports Staggered Spin-up (SSS) - RO: When set to ‘1’, the HBA supports staggered
	 * spin-up on its ports, for use in balancing power spikes. When cleared to ‘0’, this
	 * function is not supported. This value is loaded by the BIOS prior to OS initiallization.
	 */
	DWORD SSSU:1;
	/**
	 * Supports Mechanical Presence Switch (SMPS) - RO: When set to ‘1’, the HBA supports
	 * mechanical presence switches on its ports for use in hot plug operations. When
	 * cleared to ‘0’, this function is not supported. This value is loaded by the BIOS prior to
	 * OS initialization.
	 */
	DWORD SMPS:1;
	/**
	 * Supports SNotification Register (SSNTF) - RO: When set to ‘1’, the
	 * HBA supports the PxSNTF (SNotification) register and its associated functionality.
	 * When cleared to ‘0’, the HBA does not support the PxSNTF (SNotification) register and
	 * its associated functionality. Refer to section 10.11.1. Asynchronous notification with a
	 * directly attached device is always supported.
	 */
	DWORD SSNT:1;
	/**
	 * Supports Native Command Queuing (SNCQ) - RO: Indicates whether the HBA supports
	 * Serial ATA native command queuing. If set to ‘1’, an HBA shall handle DMA Setup
	 * FISes natively, and shall handle the auto-activate optimization through that FIS. If
	 * cleared to ‘0’, native command queuing is not supported and software should not issue
	 * any native command queuing commands.
	 */
	DWORD SNCQ:1;
	/**
	 * Supports 64-bit Addressing (S64A) - RO: Indicates whether the HBA can access 64-bit
	 * data structures. When set to ‘1’, the HBA shall make the 32-bit upper bits of the port
	 * DMA Descriptor, the PRD Base, and each PRD entry read/write. When cleared to ‘0’,
	 * these are read-only and treated as ‘0’ by the HBA.
	 */
	DWORD S64A:1;

	/**
	 * HBA Reset (HR) - RW: When set by SW, this bit causes an internal reset of the HBA. All
	 * state machines that relate to data transfers and queuing shall return to an idle
	 * condition, and all ports shall be re-initialized via COMRESET (if staggered spin-up is
	 * not supported). If staggered spin-up is supported, then it is the responsibility of
	 * software to spin-up each port after the reset has completed.
	 *
	 * When the HBA has performed the reset action, it shall reset this bit to ‘0’. A software
	 * write of ‘0’ shall have no effect. For a description on which bits are reset when this bit is
	 * set, see section 10.4.3.
	 */
	DWORD HRST:1;
	/**
	 * Interrupt Enable (IE) - RW: This global bit enables interrupts from the HBA. When cleared
	 * (reset default), all interrupt sources from all ports are disabled. When set, interrupts are
	 * enabled.
	 */
	DWORD INTE:1;
	/**
	 * MSI Revert to Single Message (MRSM) - RO: When set to ‘1’ by hardware, indicates that
	 * the HBA requested more than one MSI vector but has reverted to using the first vector
	 * only. When this bit is cleared to ‘0’, the HBA has not reverted to single MSI mode (i.e.
	 * hardware is already in single MSI mode, software has allocated the number of
	 * messages requested, or hardware is sharing interrupt vectors if MC.MME < MC.MMC).
	 *
	 * The HBA may revert to single MSI mode when the number of vectors allocated by the
	 * host is less than the number requested. This bit shall only be set to ‘1’ when the
	 * following conditions hold:
	 * -  MC.MSIE = ‘1’ (MSI is enabled)
	 * -  MC.MMC > 0 (multiple messages requested)
	 * -  MC.MME > 0 (more than one message allocated)
	 * -  MC.MME != MC.MMC (messages allocated not equal to number requested)
	 *
	 * When this bit is set to ‘1’, single MSI mode operation is in use and software is
	 * responsible for clearing bits in the IS register to clear interrupts.
	 *
	 * This bit shall be cleared to ‘0’ by hardware when any of the four conditions stated is
	 * false. This bit is also cleared to ‘0’ when MC.MSIE = ‘1’ and MC.MME = 0h. In this
	 * case, the hardware has been programmed to use single MSI mode, and is not
	 * “reverting” to that mode.
	 */
	DWORD MRSM:1;
	DWORD RSV1:28;
	/**
	 * AHCI Enable (AE) - RW/RO: When set, indicates that communication to the HBA shall be via
	 * AHCI mechanisms. This can be used by an HBA that supports both legacy
	 * mechanisms (such as SFF-8038i) and AHCI to know when the HBA is running under an
	 * AHCI driver.
	 *
	 * When set, software shall only communicate with the HBA using AHCI. When cleared,
	 * software shall only communicate with the HBA using legacy mechanisms. When
	 * cleared FISes are not posted to memory and no commands are sent via AHCI
	 * mechanisms.
	 *
	 * Software shall set this bit to ‘1’ before accessing other AHCI registers. When software
	 * clears this bit to ‘0’ from a previous value of ‘1’, it shall set no other bit in the GHC
	 * register as part of that operation (i.e., clearing the AE bit requires software to write
	 * 00000000h to the register).
	 *
	 * The implementation of this bit is dependent upon the value of the CAP.SAM bit. If
	 * CAP.SAM is '0', then GHC.AE shall be read-write and shall have a reset value of '0'. If
	 * CAP.SAM is '1', then AE shall be read-only and shall have a reset value of '1'.
	 */
	DWORD AHCI:1;

	/**
	 * Interrupt Pending Status (IPS) - RW1C: If set, indicates that the corresponding port has an
	 * interrupt pending. Software can use this information to determine which ports require
	 * service after an interrupt.
	 *
	 * The IPS[x] bit is only defined for ports that are implemented or for the command
	 * completion coalescing interrupt defined by CCC_CTL.INT. All other bits are reserved.
	 */
	DWORD ISTS;

	/**
	 * Port Implemented (PI) - RO: This register is bit significant. If a bit is set to ‘1’, the
	 * corresponding port is available for software to use. If a bit is cleared to ‘0’, the port is
	 * not available for software to use. The maximum number of bits set to ‘1’ shall not
	 * exceed CAP.NP + 1, although the number of bits set in this register may be fewer than
	 * CAP.NP + 1. At least one bit shall be set to ‘1’.
	 */
	DWORD PIMP;

	/**
	 * This register indicates the major and minor version of the AHCI specification that the HBA implementation
	 * supports. The upper two bytes represent the major version number, and the lower two bytes represent
	 * the minor version number. Example: Version 3.12 would be represented as 00030102h. Three versions
	 * of the specification are valid: 0.95, 1.0, 1.1, 1.2, 1.3, and 1.3.1.
	 */
	DWORD VERS;

	/**
	 * Enable (EN) - RW: When cleared to ‘0’, the command completion coalescing feature is
	 * disabled and no CCC interrupts are generated. When set to ‘1’, the command
	 * completion coalescing feature is enabled and CCC interrupts may be generated based
	 * on timeout or command completion conditions. Software shall only change the contents
	 * of the TV and CC fields when EN is cleared to ‘0’. On transition of this bit from ‘0’ to ‘1’,
	 * any updated values for the TV and CC fields shall take effect.
	 */
	DWORD ENAL:1;
	DWORD RSV2:2;
	/**
	 * Interrupt (INT) - RO: Specifies the interrupt used by the CCC feature. This interrupt must be
	 * marked as unused in the Ports Implemented (PI) register by the corresponding bit being
	 * set to ‘0’. Thus, the CCC interrupt corresponds to the interrupt for an unimplemented
	 * port on the controller. When a CCC interrupt occurs, the IS.IPS[INT] bit shall be
	 * asserted to ‘1’. This field also specifies the interrupt vector used for MSI.
	 */
	DWORD INTT:5;
	/**
	 * Command Completions (CC) - RW: Specifies the number of command completions that are
	 * necessary to cause a CCC interrupt. The HBA has an internal command completion
	 * counter, hCccComplete. hCccComplete is incremented by one each time a selected
	 * port has a command completion. When hCccComplete is equal to the command
	 * completions value, a CCC interrupt is signaled. The internal command completion
	 * counter is reset to ‘0’ on the assertion of each CCC interrupt. A value of ‘0’ for this field
	 * shall disable CCC interrupts being generated based on the number of commands
	 * completed, i.e. CCC interrupts are only generated based on the timer in this case.
	 */
	DWORD CCMP:8;
	/**
	 * Timeout Value (TV) - RW: The timeout value is specified in 1 millisecond intervals. The timer
	 * accuracy shall be within 5%. hCccTimer is loaded with this timeout value. hCccTimer is
	 * only decremented when commands are outstanding on selected ports, as defined in
	 * section 11.2. The HBA will signal a CCC interrupt when hCccTimer has decremented to
	 * ‘0’. hCccTimer is reset to the timeout value on the assertion of each CCC interrupt. A
	 * timeout value of ‘0’ is reserved.
	 */
	DWORD TMOV:16;

	/**
	 * Ports (PRT): This register is bit significant. Each bit corresponds to a particular port,
	 * where bit 0 corresponds to port 0. If a bit is set to ‘1’, the corresponding port is part of
	 * the command completion coalescing feature. If a bit is cleared to ‘0’, the port is not part
	 * of the command completion coalescing feature. Bits set to ‘1’ in this register must also
	 * have the corresponding bit set to ‘1’ in the Ports Implemented register. An updated
	 * value for this field shall take effect within one timer increment (1 millisecond).
	 */
	DWORD CCCP;

	/**
	 * Buffer Size (SZ): Specifies the size of the transmit message buffer area in Dwords. If
	 * both transmit and receive buffers are supported, then the transmit buffer begins at
	 * ABAR[EM_LOC.OFST*4] and the receive buffer directly follows it. If both transmit and
	 * receive buffers are supported, both buffers are of the size indicated in the Buffer Size
	 * field. A value of ‘0’ is invalid.
	 */
	DWORD BFSZ:16;
	// Offset (OFST) - RW: The offset of the message buffer in Dwords from the beginning of the ABAR.
	DWORD OFST:16;

	DWORD EMCT;
	DWORD XCAP;
	DWORD BOHC;

	DWORD RSV3[0x35];

	AHCI_CONTROLLER_PORT PORT[];
} AHCI_CONTROLLER_GENERAL;
typedef struct _PCIE_AHCI_CONTROLLER PCIE_AHCI_CONTROLLER;
struct _PCIE_AHCI_CONTROLLER
{
	PCI_EXPRESS_DEVICE       PCIE;
	PCIE_AHCI_CONTROLLER    *NEXT;
	AHCI_CONTROLLER_GENERAL *AHBA;
	REENTRANT_LOCK           LOCK[32];
};
typedef struct _AHCI_COMMAND_HEADER
{
	/**
	 * Command FIS Length (CFL): Length of the Command FIS. A ‘0’ represents 0 DW, ‘4’
	 * represents 4 DW. A length of ‘0’ or ‘1’ is illegal. The maximum value allowed is 10h, or 16 DW.
	 * The HBA uses this field to know the length of the FIS it shall send to the device.
	 */
	WORD  CFIL:5;
	/**
	 * ATAPI (A): When ‘1’, indicates that a PIO setup FIS shall be sent by the device indicating a
	 * transfer for the ATAPI command. The HBA may prefetch data from CTBAz[ACMD] in
	 * anticipation of receiving the PIO Setup FIS.
	 */
	WORD  ATAP:1;
	/**
	 * Write (W): When set, indicates that the direction is a device write (data from system memory to
	 * device). When cleared, indicates that the direction is a device read (data from device to system
	 * memory). If this bit is set and the P bit is set, the HBA may prefetch data in anticipation of
	 * receiving a DMA Setup FIS, a DMA Activate FIS, or PIO Setup FIS, in addition to prefetching
	 * PRDs.
	 */
	WORD  WRIT:1;
	/**
	 * Prefetchable (P): This bit is only valid if the PRDTL field is non-zero or the ATAPI ‘A’ bit is set
	 * in the command header. When set and PRDTL is non-zero, the HBA may prefetch PRDs in
	 * anticipation of performing a data transfer. When set and the ATAPI ‘A’ bit is set in the command
	 * header, the HBA may prefetch the ATAPI command. System software shall not set this bit when
	 * using native command queuing commands or when using FIS-based switching with a Port
	 * Multiplier.
	 *
	 * Note: The HBA may prefetch the ATAPI command, PRD entries, and data regardless of the
	 * state of this bit. However, it is recommended that the HBA use this information from software to
	 * avoid prefetching needlessly.
	 */
	WORD  PFET:1;
	/**
	 * Reset (R): When ‘1’, indicates that the command that software built is for a part of a software
	 * reset sequence that manipulates the SRST bit in the Device Control register. The HBA must
	 * perform a SYNC escape (if necessary) to get the device into an idle state before sending the
	 * command. See section 10.4 for details on reset.
	 */
	WORD  RSET:1;
	/**
	 * BIST (B): When ‘1’, indicates that the command that software built is for sending a BIST FIS.
	 * The HBA shall send the FIS and enter a test mode. The tests that can be run in this mode are
	 * outside the scope of this specification.
	 */
	WORD  BIST:1;
	/**
	 * Clear Busy upon R_OK (C): When set, the HBA shall clear PxTFD.STS.BSY and
	 * PxCI.CI(pIssueSlot) after transmitting this FIS and receiving R_OK. When cleared, the HBA
	 * shall not clear PxTFD.STS.BSY nor PxCI.CI(pIssueSlot) after transmitting this FIS and receiving
	 * R_OK.
	 */
	WORD  CBOK:1;
	WORD  RSV0:1;
	/**
	 * Port Multiplier Port (PMP): Indicates the port number that should be used when constructing
	 * Data FISes on transmit, and to check against all FISes received for this command. This value
	 * shall be set to 0h by software when it has been determined that it is communicating to a directly
	 * attached device.
	 */
	WORD  PMUP:4;
	/**
	 * Physical Region Descriptor Table Length (PRDTL): Length of the scatter/gather descriptor
	 * table in entries, called the Physical Region Descriptor Table. Each entry is 4 DW. A ‘0’
	 * represents 0 entries, FFFFh represents 65,535 entries. The HBA uses this field to know when to
	 * stop fetching PRDs. If this field is ‘0’, then no data transfer shall occur with the command.
	 */
	WORD  RDTL;

	/**
	 * Physical Region Descriptor Byte Count (PRDBC): Indicates the current byte count that has
	 * transferred on device writes (system memory to device) or device reads (device to system
	 * memory).
	 *
	 * For rules on when this field is updated, refer to section 5.4.1
	 */
	DWORD RDBC;

	/**
	 * Command Table Descriptor Base Address (CTBA): Indicates the 32-bit physical address of
	 * the command table, which contains the command FIS, ATAPI Command, and PRD table. This
	 * address must be aligned to a 128-byte cache line, indicated by bits 06:00 being reserved.
	 */
	QWORD CTBA;

	DWORD RSV1[4];
} AHCI_COMMAND_HEADER;
typedef struct _AHCI_PRDT_ENTRY
{
	// Data Base Address (DBA): Indicates the 32-bit physical address of the data block. The block must be word aligned, indicated by bit 0 being reserved.
	QWORD DATA;
	DWORD RSV0;
	/**
	 * Data Byte Count (DBC): A ‘0’ based value that Indicates the length, in bytes, of the data block.
	 * A maximum of length of 4MB may exist for any entry. Bit ‘0’ of this field must always be ‘1’ to
	 * indicate an even byte count. A value of ‘1’ indicates 2 bytes, ‘3’ indicates 4 bytes, etc.
	 */
	DWORD BCNT:22;
	DWORD RSV1:9;
	/**
	 * Interrupt on Completion (I): When set, indicates that hardware should assert an interrupt when
	 * the data block for this entry has transferred, which means that no data is in the HBA hardware.
	 * Data may still be in flight to system memory (disk reads), or at the device (an R_OK or R_ERR
	 * has yet to be received). The HBA shall set the PxIS.DPS bit after completing the data transfer,
	 * and if enabled, generate an interrupt.
	 */
	DWORD IONC:1;
} AHCI_PRDT_ENTRY;
typedef struct _AHCI_COMMAND_TABLE
{
	// 0x00
	/**
	 * This is a software constructed FIS. For data transfer operations, this is the H2D Register FIS format as
	 * specified in the Serial ATA Revision 2.6 specification. The HBA sets PxTFD.STS.BSY, and then sends
	 * this structure to the attached port. If a Port Multiplier is attached, this field must have the Port Multiplier
	 * port number in the FIS itself – it shall not be added by the HBA. Valid CFIS lengths are 2 to 16 Dwords
	 * and must be in Dword granularity.
	 */
	BYTE CFIS[64]; // Command FIS

	// 0x40
	/**
	 * This is a software constructed region of 12 or 16 bytes in length that contains the ATAPI command to
	 * transmit if the “A” bit is set in the command header. The ATAPI command must be either 12 or 16 bytes
	 * in length. The length transmitted by the HBA is determined by the PIO setup FIS that is sent by the
	 * device requesting the ATAPI command.
	 */
	BYTE ACMD[16]; // ATAPI Command

	// 0x50
	BYTE RSV0[48];

	// 0x80
	/**
	 * This table contains the scatter / gather list for the data transfer. It contains a list of 0 (no data to transfer)
	 * to up to 65,535 entries. A breakdown of each field in a PRD table is shown below. Item 0 refers to the
	 * first entry in the PRD table. Item “CH[PRDTL] – 1” refers to the last entry in the table, where the length
	 * field comes from the PRDTL field in the command list entry for this command slot.
	 */
	AHCI_PRDT_ENTRY PRDT[];
} AHCI_COMMAND_TABLE;
typedef struct _AHCI_FIS_REG_H2D
{
	// DWORD 0
	BYTE  TYPE;   // FIS_TYPE_REG_H2D
	BYTE  PMUL:4; // Port multiplier
	BYTE  RSV0:3; // Reserved
	BYTE  CORC:1; // 1: Command, 0: Control
	BYTE  CMMD;   // Command register
	BYTE  FEA0;   // Feature register, 7:0

	// DWORD 1
	BYTE  LBA0;   // LBA low register, 7:0
	BYTE  LBA1;   // LBA mid register, 15:8
	BYTE  LBA2;   // LBA high register, 23:16
	BYTE  DEVC;   // Device register

	// DWORD 2
	BYTE  LBA3;   // LBA register, 31:24
	BYTE  LBA4;   // LBA register, 39:32
	BYTE  LBA5;   // LBA register, 47:40
	BYTE  FEA1;   // Feature register, 15:8

	// DWORD 3
	WORD  CONT;   // Count register
	BYTE  ISCC;   // Isochronous command completion
	BYTE  CTRL;   // Control register

	// DWORD 4
	DWORD AUXY;   // Auxiliary

	BYTE  RSV1[44];
} AHCI_FIS_REG_H2D;

void setup_ahci_controller(PCI_EXPRESS_DEVICE *device);
void ahci_port_reset(AHCI_CONTROLLER_PORT *port);