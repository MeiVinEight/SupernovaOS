#ifndef SUPERNOVA_PCIE_H
#define SUPERNOVA_PCIE_H

#include <acpi/acpi.h>
#include <interrupt/interrupt.h>

#define PCIE_CAID_MSI  0x05
#define PCIE_CAID_MSIX 0x11

typedef struct _PCIE_SEGMENT_ADDRESS
{
	QWORD ECAM;
	WORD  PSEG;
	BYTE  SBUS;
	BYTE  EBUS;
	DWORD RSV0;
} PCIE_SEGMENT_ADDRESS;
#pragma pack(push, 4)
typedef struct _ACPI_MCFG
{
	ACPI_SDT_HEADER HEAD;
	DWORD RSV0[2];
	PCIE_SEGMENT_ADDRESS ECAM[];
} ACPI_MCFG;
#pragma pack(pop)
typedef struct _PCI_EXPRESS_CAPABILITY
{
	/**
	 * Capability ID [RO] - Indicates the PCI Express Capability structure. This field must return a Capability ID of
	 * 10h indicating that this is a PCI Express Capability structure.
	 */
	BYTE CAID;
	/**
	 * Next Capability Pointer [RO] - This field contains the offset to the next PCI Capability structure or 00h if no
	 * other items exist in the linked list of Capabilities.
	 */
	BYTE NEXT;
	/**
	 * Capability Version - Indicates PCI-SIG defined PCI Express Capability structure version number.
	 *
	 * A version of the specification that changes the PCI Express Capability structure in a way that is not
	 * otherwise identifiable (e.g., through a new Capability field) is permitted to increment this field. All such
	 * changes to the PCI Express Capability structure must be software-compatible. Software must check for
	 * Capability Version numbers that are greater than or equal to the highest number defined when the
	 * software is written, as Functions reporting any such Capability Version numbers will contain a PCI
	 * Express Capability structure that is compatible with that piece of software.
	 * Must be hardwired to 2h for Functions compliant to this specification.
	 */
	WORD CVER:4;
	/**
	 * Device/Port Type 179 - Indicates the specific type of this PCI Express Function. Note that different
	 * Functions in a Multi-Function Device can generally be of different types.
	 * Defined encodings for Functions that implement a Type 00h PCI Configuration Space header are:
	 * - 0000b PCI Express Endpoint
	 * - 0001b Legacy PCI Express Endpoint
	 * - 1001b RCiEP
	 * - 1010b Root Complex Event Collector
	 *
	 * Defined encodings for Functions that implement a Type 01h PCI Configuration Space header are:
	 * - 0100b Root Port of PCI Express Root Complex
	 * - 0101b Upstream Port of PCI Express Switch
	 * - 0110b Downstream Port of PCI Express Switch
	 * - 0111b PCI Express to PCI/PCI-X Bridge
	 * - 1000b PCI/PCI-X to PCI Express Bridge
	 *
	 * All other encodings are Reserved.
	 *
	 * Note that the different Endpoint types have notably different requirements in § Section 1.3.2 regarding
	 * I/O resources, Extended Configuration Space, and other capabilities.
	 */
	WORD DTYP:4;
	/**
	 * Slot Implemented - When Set, this bit indicates that the Link associated with this Port is connected to a
	 * slot (as compared to being connected to a system-integrated device or being disabled).
	 * This bit is valid for Downstream Ports. This bit is undefined for Upstream Ports.
	 */
	WORD SIMP:1;
	/**
	 * Interrupt Message Number - When MSI/MSI-X is implemented, this field indicates which MSI/MSI-X
	 * vector is used for the interrupt message generated in association with any of the status bits of this
	 * Capability structure. This vector is also used for:
	 * - Native PME Software Model (see § Section 6.1.6 and § Section 6.7.3.4 )
	 * - Link Activation (see § Section 5.5.6 )
	 * - Flit Error Counter Interrupts (see § Section 7.7.9.4 )
	 *
	 * For MSI, the value in this field indicates the offset between the base Message Data and the interrupt
	 * message that is generated. Hardware is required to update this field so that it is correct if the number of
	 * MSI Messages assigned to the Function changes when software writes to the Multiple Message Enable
	 * field in the Message Control Register for MSI.
	 *
	 * For MSI-X, the value in this field indicates which MSI-X Table entry is used to generate the interrupt
	 * message. The entry must be one of the first 32 entries even if the Function implements more than 32
	 * entries. For a given MSI-X implementation, the entry must remain constant.
	 *
	 * If both MSI and MSI-X are implemented, they are permitted to use different vectors, though software is
	 * permitted to enable only one mechanism at a time. If MSI-X is enabled, the value in this field must
	 * indicate the vector for MSI-X. If MSI is enabled or neither is enabled, the value in this field must indicate
	 * the vector for MSI. If software enables both MSI and MSI-X at the same time, the value in this field is
	 * undefined.
	 */
	WORD IMNU:5;
	WORD RSV0:1;
	/**
	 * Flit Mode Supported – When Set, indicates support for Flit Mode. Must be Set by all implementations
	 * that support Flit Mode. Must be Clear by implementations that do not support Flit Mode. See Flit Mode
	 * Supported bit.
	 */
	WORD FLIT:1;
	/**
	 * Max_Payload_Size Supported - This field indicates the maximum payload size that the Function can
	 * support for TLPs. This field MUST@FLIT indicate a minimum of 512 bytes.
	 *
	 * If the Rx_MPS_Fixed bit is Set, the Function's Rx_MPS_Limit is fixed with the value indicated by this
	 * (Max_Payload_Size Supported) field. Otherwise, the Rx_MPS_Limit is determined by the
	 * Max_Payload_Size field (the "MPS setting") in one or more Functions. See § Section 2.2.2 for important
	 * details regarding Multi-Function Devices.
	 * Defined encodings are:
	 * - 000b 128 bytes max payload size
	 * - 001b 256 bytes max payload size
	 * - 010b 512 bytes max payload size
	 * - 011b 1024 bytes max payload size
	 * - 100b 2048 bytes max payload size
	 * - 101b 4096 bytes max payload size
	 * - 110b Reserved
	 * - 111b Reserved
	 *
	 * The Functions of a Multi-Function Device are permitted to report different values for this field.
	 */
} PCI_EXPRESS_CAPABILITY;
typedef struct _PCI_CONFIGURATION_INTERRUPT
{
	BYTE line;
	BYTE pin;
	BYTE grant;
	BYTE latency;
} PCI_CONFIGURATION_INTERRUPT;
typedef struct _PCI_CONFIGURATION_SPACE
{
	WORD vendor;
	WORD device;
	WORD command;
	WORD status;
	DWORD revision:8;
	DWORD class:24;
	BYTE cache;
	BYTE latency;
	BYTE type;
	BYTE bist; // Built-In Self-Test
	DWORD address[6];
	DWORD cardbus;
	DWORD subsystem;
	DWORD expansion;
	BYTE capability;
	BYTE resvered0[7];
	PCI_CONFIGURATION_INTERRUPT interrupt;
} PCI_CONFIGURATION_SPACE;
typedef struct _PCI_EXPRESS_DEVICE
{
	DWORD offset:8;
	DWORD function:3;
	DWORD device:5;
	DWORD bus:8;
	DWORD reserve:8;
	volatile PCI_CONFIGURATION_SPACE *configuration;
} PCI_EXPRESS_DEVICE;

extern volatile ACPI_MCFG *volatile MCFG;

void setup_pcie_mcfg(ACPI_MCFG *mcfg);
void setup_pcie();
QWORD pcie_cfg_get_base_address(PCI_EXPRESS_DEVICE *device, DWORD addrIdx);
DWORD pcie_bar_count(PCI_EXPRESS_DEVICE *device);
PCI_EXPRESS_CAPABILITY *__stdcall pcie_capability(PCI_EXPRESS_DEVICE *device, DWORD capa);
DWORD pcie_setup_interrupt(PCI_EXPRESS_DEVICE *device, void (*irq)(INTERRUPT_STACK *stack), BYTE intx);
void pcie_enable_bus_master(PCI_EXPRESS_DEVICE *device);

#endif
