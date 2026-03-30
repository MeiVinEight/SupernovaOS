//
// Created by MeiVi on 2026/03/23.
//

#ifndef SUPERNOVA_XHC_REGS_H
#define SUPERNOVA_XHC_REGS_H

#include <types.h>

#define XHCI_USBSTS_EINT (1 << 3)

/**
 * This capability provides the xHCI Pre-OS to OS
 *Handoff Synchronization support capability.
 */
#define XHCI_XECP_USB_LATENCY_SUPPORT        1
/**
 * This capability enumerates the protocols and
 * revisions supported by this xHC. At least one of
 * these capability structures is required for all xHC
 * implementations.
 */
#define XHCI_XECP_SUPPORTED_PROTOCOL         2
/**
 * This capability is required for all xHC non-PCI
 * implementations.
 */
#define XHCI_XECP_EXTENDED_POWER_MANAGEMENT  3
/**
 * This capability is optional-normative for xHC
 * implementations that require hardware
 * virtualization support.
 */
#define XHCI_XECP_IO_VIRTUALIZATION          4
/**
 * Either this or the xHCI Extended Message
 * Interrupt capability is required for all xHC nonPCI implementations.
 */
#define XHCI_XECP_MESSAGE_INTERRUPT          5
/**
 * This capability is optional-normative for xHC
 * implementations that require local memory
 * support.
 */
#define XHCI_XECP_LOCAL_MEMORY               6
/**
 * This capability is optional-normative for xHC
 * implementations and describes the xHCI USB
 * Debug Capability.
 */
#define XHCI_XECP_USB_DEBUG_CAPABILITY       10
/**
 * Either this or the xHCI Message Interrupt
 * capability is required for all xHC non-PCI
 * implementations.
 */
#define XHCI_XECP_EXTENDED_MESSAGE_INTERRUPT 17

typedef struct _XHCI_CAPABILITY_SPACE
{
	BYTE SIZE;
	BYTE RSV0;
	WORD VRSN;
	//DWORD STR1;
	DWORD SLOT:8; // Max Number of Device Slots
	DWORD INTE:11;
	DWORD RSV1:5;
	DWORD PORT:8;
	//DWORD STR2;
	DWORD ISTH:4;
	/**
	 * Event Ring Segment Table Max (ERST Max). Default = implementation dependent. Valid values
	 * are 0 – 15. This field determines the maximum value supported the Event Ring Segment Table
	 * Base Size registers (5.5.2.3.1), where:
	 *  The maximum number of Event Ring Segment Table entries = 2 ERST Max
	 * .
	 * e.g. if the ERST Max = 7, then the xHC Event Ring Segment Table(s) supports up to 128 entries,
	 * 15 then 32K entries, etc.
	 */
	DWORD ERST:4;
	DWORD RSV2:13;
	DWORD MSBH:5;
	DWORD SPDR:1;
	DWORD MSBL:5;
	DWORD STR3;
	//DWORD CAP1;
	DWORD AC64:1;
	DWORD BWNC:1;
	DWORD CX64:1;
	DWORD PPWC:1;
	DWORD PIND:1;
	DWORD LHCR:1;  // Light Host Controller Reset
	DWORD LTMC:1;  // Latency Tolerance Messaging Capability
	DWORD NSSS:1;  // No Secondary SID Support
	DWORD PAED:1;  // Parse All Event Data
	DWORD SSPC:1;  // Stopped - Short Packet Capability
	DWORD SEDC:1;  // Stopped EDTLA Capability
	DWORD CFIC:1;  // Contiguous Frame ID Capability
	DWORD PSAS:4;  // Maximum Primary Stream Array Size
	/**
	 * xHCI Extended Capabilities Pointer (xECP). This field indicates the existence of a capabilities list.
	 * The value of this field indicates a relative offset, in 32-bit words, from Base to the beginning of
	 * the first extended capability.
	 *
	 * For example, using the offset of Base is 1000h and the xECP value of 0068h, we can calculated
	 * the following effective address of the first extended capability:
	 * 1000h + (0068h << 2) -> 1000h + 01A0h -> 11A0h
	 */
	DWORD XECP:16;
	/**
	 *This register defines the offset of the Doorbell Array base address from the
	 * Base.
	 *
	 * Doorbell Array Offset - RO. Default = implementation dependent. This field defines the offset
	 * in Dwords of the Doorbell Array base address from the Base (i.e. the base address of the xHCI
	 * Capability register address space).
	 *
	 * Note: Normally the Doorbell Array is Dword aligned, however if virtualization is
	 * supported by the xHC (either through IOV or VTIO) then it shall be PAGESIZE
	 * aligned. e.g. If the PAGESIZE = 4K (1000h), and the Doorbell Array is positioned
	 * at a 3 page offset from the Base, then this register shall report 0000 3000h.
	 */
	DWORD BELL;    // Doorbell Offset
	DWORD RTME;    // Runtime Offset
	DWORD CAP2;
} XHCI_CAPABILITY_SPACE;
typedef struct _XHCI_OPERATIONAL_SPACE
{
	//DWORD command;
	/**
	 * Run/Stop (R/S) – RW. Default = ‘0’. ‘1’ = Run. ‘0’ = Stop. When set to a ‘1’, the xHC proceeds with
	 * execution of the schedule. The xHC continues execution as long as this bit is set to a ‘1’. When
	 * this bit is cleared to ‘0’, the xHC completes any current or queued commands or TDs, and any
	 * USB transactions associated with them, then halts.
	 *
	 * Refer to section 5.4.1.1 for more information on how R/S shall be managed.
	 *
	 * The xHC shall halt within 16 ms. after software clears the Run/Stop bit if the above conditions
	 * have been met.
	 *
	 * The HCHalted (HCH) bit in the USBSTS register indicates when the xHC has finished its pending
	 * pipelined transactions and has entered the stopped state. Software shall not write a ‘1’ to this
	 * flag unless the xHC is in the Halted state (i.e. HCH in the USBSTS register is ‘1’). Doing so may
	 * yield undefined results. Writing a ‘0’ to this flag when the xHC is in the Running state (i.e. HCH =
	 * ‘0’) and any Event Rings are in the Event Ring Full state (refer to section 4.9.4) may result in lost
	 * events.
	 *
	 * When this register is exposed by a Virtual Function (VF), this bit only controls the run state of
	 * the xHC instance presented by the selected VF. Refer to section 8 for more information.
	 */
	DWORD RNST:1;
	/**
	 * Host Controller Reset (HCRST) – RW. Default = ‘0’. This control bit is used by software to reset
	 * the host controller. The effects of this bit on the xHC and the Root Hub registers are similar to a
	 * Chip Hardware Reset.
	 *
	 * When software writes a ‘1’ to this bit, the Host Controller resets its internal pipelines, timers,
	 * counters, state machines, etc. to their initial value. Any transaction currently in progress on the
	 * USB is immediately terminated. A USB reset shall not be driven on USB2 downstream ports,
	 * however a Hot or Warm Reset79 shall be initiated on USB3 Root Hub downstream ports.
	 *
	 * PCI Configuration registers are not affected by this reset. All operational registers, including port
	 * registers and port state machines are set to their initial values. Software shall reinitialize the
	 * host controller as described in Section 4.2 in order to return the host controller to an
	 * operational state.
	 *
	 * This bit is cleared to ‘0’ by the Host Controller when the reset process is complete. Software
	 * cannot terminate the reset process early by writing a ‘0’ to this bit and shall not write any xHC
	 * Operational or Runtime registers until while HCRST is ‘1’. Note, the completion of the xHC reset
	 * process is not gated by the Root Hub port reset process.
	 *
	 * Software shall not set this bit to ‘1’ when the HCHalted (HCH) bit in the USBSTS register is a ‘0’.
	 * Attempting to reset an actively running host controller may result in undefined behavior.
	 *
	 * When this register is exposed by a Virtual Function (VF), this bit only resets the xHC instance
	 * presented by the selected VF. Refer to section 8 for more information.
	 */
	DWORD HRST:1;
	/**
	 * Interrupter Enable (INTE) – RW. Default = ‘0’. This bit provides system software with a means of
	 * enabling or disabling the host system interrupts generated by Interrupters. When this bit is a ‘1’,
	 * then Interrupter host system interrupt generation is allowed, e.g. the xHC shall issue an interrupt
	 * at the next interrupt threshold if the host system interrupt mechanism (e.g. MSI, MSI-X, etc.) is
	 * enabled. The interrupt is acknowledged by a host system interrupt specific mechanism.
	 *
	 * When this register is exposed by a Virtual Function (VF), this bit only enables the set of
	 * Interrupters assigned to the selected VF. Refer to section 7.7.2 for more information.
	 */
	DWORD INTE:1;
	DWORD HSSE:1; // Host System Error Enable
	DWORD RSV0:3;
	DWORD LHCR:1; // Light Host Controller Reset
	DWORD COSS:1; // Controller Save Status
	DWORD CORS:1; // Controller Resotre Status
	DWORD ENWE:1; // Enable Wrap Event
	DWORD EU3S:1; // Enable U3 MFINDEX Stop
	DWORD RSV1:1;
	DWORD ECEM:1; // Enable CEM
	DWORD ETBC:1; // Enable Extended TBC
	DWORD ETTS:1; // Enable Extended TBC TRB Status
	DWORD VTIO:1; // Enable VTIO
	DWORD RSV2:15;
	union
	{
		DWORD USTS;
		struct
		{
			/**
			 * HCHalted (HCH) – RO. Default = ‘1’. This bit is a ‘0’ whenever the Run/Stop (R/S) bit is a ‘1’. The
			 * xHC sets this bit to ‘1’ after it has stopped executing as a result of the Run/Stop (R/S) bit being
			 * cleared to ‘0’, either by software or by the xHC hardware (e.g. internal error).
			 *
			 * If this bit is '1', then SOFs, microSOFs, or Isochronous Timestamp Packets (ITP) shall not be
			 * generated by the xHC, and any received Transaction Packet shall be dropped.
			 *
			 * When this register is exposed by a Virtual Function (VF), this bit only reflects the Halted state of
			 * the xHC instance presented by the selected VF. Refer to section 8 for more information.
			 */
			DWORD HALT:1;
			DWORD RSV3:1;
			DWORD SYSE:1; // Host System Error
			/**
			 * Event Interrupt (EINT) – RW1C. Default = ‘0’. The xHC sets this bit to ‘1’ when the Interrupt
			 * Pending (IP) bit of any Interrupter transitions from ‘0’ to ‘1’. Refer to section 7.1.2 for use.
			 * Software that uses EINT shall clear it prior to clearing any IP flags. A race condition may occur if
			 * software clears the IP flags then clears the EINT flag, and between the operations another IP ‘0’
			 * to '1' transition occurs. In this case the new IP transition shall be lost.
			 * When this register is exposed by a Virtual Function (VF), this bit is the logical 'OR' of the IP bits
			 * for the Interrupters assigned to the selected VF. And it shall be cleared to ‘0’ when all associated
			 * interrupter IP bits are cleared, i.e. all the VF’s Interrupter Event Ring(s) are empty. Refer to
			 * section 8 for more information.
			 */
			DWORD EINT:1;
			DWORD PCDT:1; // Port Change Detect
			DWORD RSV4:3;
			DWORD SSST:1; // Save State Status
			DWORD RSST:1; // Restore State Status
			DWORD SRER:1; // Save/Restore Error
			/**
			 * Controller Not Ready (CNR) – RO. Default = ‘1’. ‘0’ = Ready and ‘1’ = Not Ready. Software shall
			 * not write any Doorbell or Operational register of the xHC, other than the USBSTS register, until
			 * CNR = ‘0’. This flag is set by the xHC after a Chip Hardware Reset and cleared when the xHC is
			 * ready to begin accepting register writes. This flag shall remain cleared (‘0’) until the next Chip
			 * Hardware Reset.
			 */
			DWORD HCNR:1;
			DWORD CTRE:1; // Host Controller Error
			DWORD RSV5:19;
		};
	};
	DWORD PAGE;
	DWORD RSV6[2];
	DWORD DNCR; // Device Notification Control Register
	QWORD CRCR;
	DWORD RSV7[4];
	QWORD CBAA;
	//DWORD configure;
	DWORD MDSE:8; // Max Device Slots Enabled
	DWORD ENU3:1; // Enable U3 Entry
	DWORD ENCI:1; // Enable Configuration Information
	DWORD RSV8:22;
	DWORD RSV9[0xF1];
} XHCI_OPERATIONAL_SPACE;
static_assert(sizeof(XHCI_OPERATIONAL_SPACE) == 1024, "Wrong xHCI Operation structural");
typedef struct _XHCI_INTERRUPTER
{
	/**
	 * Interrupt Pending (IP) - RW1C. Default = ‘0’. This flag represents the current state of the
	 * Interrupter. If IP = ‘1’, an interrupt is pending for this Interrupter. A ‘0’ value indicates that no
	 * interrupt is pending for the Interrupter. Refer to section 4.17.3 for the conditions that modify
	 * the state of this flag.
	 */
	DWORD IPEN:1; // Interrupt Pending
	/**
	 * Interrupt Enable (IE) – RW. Default = ‘0’. This flag specifies whether the Interrupter is capable of
	 * generating an interrupt. When this bit and the IP bit are set (‘1’), the Interrupter shall generate
	 * an interrupt when the Interrupter Moderation Counter reaches ‘0’. If this bit is ‘0’, then the
	 * Interrupter is prohibited from generating interrupts.
	 */
	DWORD IENA:1;
	DWORD RSV0:30;
	/**
	 * Interrupt Moderation Interval (IMODI) – RW. Default = ‘4000’ (~1ms). Minimum inter-interrupt
	 * interval. The interval is specified in 250ns increments. A value of ‘0’ disables interrupt throttling
	 * logic and interrupts shall be generated immediately if IP = ‘0’, EHB = ‘0’, and the Event Ring is
	 * not empty.
	 */
	DWORD IMOI:16;
	/**
	 * Interrupt Moderation Counter (IMODC) – RW. Default = undefined. Down counter. Loaded with
	 * the IMODI value whenever IP is cleared to ‘0’, counts down to ‘0’, and stops. The associated
	 * interrupt shall be signaled whenever this counter is ‘0’, the Event Ring is not empty, the IE and IP
	 * flags = ‘1’, and EHB = ‘0’.
	 * This counter may be directly written by software at any time to alter the interrupt rate.
	 */
	DWORD IMOC:16;
	/**
	 * Event Ring Segment Table Size – RW. Default = ‘0’. This field identifies the number of valid
	 * Event Ring Segment Table entries in the Event Ring Segment Table pointed to by the Event Ring
	 * Segment Table Base Address register. The maximum value supported by an xHC
	 * implementation for this register is defined by the ERST Max field in the HCSPARAMS2 register
	 * (5.3.4).
	 * For Secondary Interrupters: Writing a value of ‘0’ to this field disables the Event Ring. Any events
	 * targeted at this Event Ring when it is disabled shall result in undefined behavior of the Event
	 * Ring.
	 * For the Primary Interrupter: Writing a value of ‘0’ to this field shall result in undefined behavior
	 * of the Event Ring. The Primary Event Ring cannot be disabled.
	 */
	DWORD STSZ:16;
	DWORD RSV1:16;
	DWORD RSV2;
	/**
	 * Event Ring Segment Table Base Address Register – RW. Default = ‘0’. This field defines the
	 * high order bits of the start address of the Event Ring Segment Table.
	 *
	 * Writing this register sets the Event Ring State Machine:EREP Advancement to the Start state.
	 * Refer to Figure 4-12 for more information.
	 *
	 * For Secondary Interrupters: This field may be modified at any time.
	 *
	 * For the Primary Interrupter: This field shall not be modified if HCHalted (HCH) = ‘0’.
	 *
	 * Aligned at 64-byte boundary.
	 */
	QWORD STBA;
	/**
	 * Dequeue ERST Segment Index (DESI) – RW. Default = ‘0’. This field may be used by the xHC to
	 * accelerate checking the Event Ring full condition. This field is written with the low order 3 bits of
	 * the offset of the ERST entry which defines the Event Ring segment that the Event Ring Dequeue
	 * Pointer resides in. Refer to section 6.5 for the definition of an ERST entry.
	 */
	QWORD DESI:3;
	/**
	 * Event Handler Busy (EHB) - RW1C. Default = ‘0’. This flag shall be set to ‘1’ when the IP bit is set
	 * to ‘1’ and cleared to ‘0’ by software when the Dequeue Pointer register is written. Refer to
	 * section 4.17.2 for more information.
	 */
	QWORD EHBS:1;
	/**
	 * Event Ring Dequeue Pointer - RW. Default = ‘0’. This field defines the high order bits of the 64-
	 * bit address of the current Event Ring Dequeue Pointer.
	 *
	 * Aligned at 16-byte boundary.
	 */
	QWORD ERDP:60;
} XHCI_INTERRUPTER;
typedef struct _XHCI_RUNTIME_SPACE
{
	/**
	 * Microframe Index – RO. The value in this register increments at the end of each microframe (e.g.
	 * 125us.). Bits [13:3] may be used to determine the current 1ms. Frame Index.
	 */
	DWORD MFRM:14;
	DWORD RSV0:18;
	DWORD RSV1[7];
	XHCI_INTERRUPTER INTR[];
} XHCI_RUNTIME_SPACE;

/**
 * Note: The Ring Segment Size may be set to any value from 16 to 4096, however
 * software shall allocate a buffer for the Event Ring Segment that rounds up its
 * size to the nearest 64B boundary to allow full cache-line accesses.
 */
typedef struct _XHCI_EVENT_RING_SEGMENT
{
	/**
	 * Ring Segment Base Address Hi and Lo. These fields represent the high order bits of the 64-bit
	 * base address of the Event Ring Segment.
	 *
	 * The memory structure referenced by this physical memory pointer shall begin on a 64-byte
	 * address boundary.
	 *
	 * Aligned at 64-byte boundary.
	 */
	QWORD RSBA;
	/**
	 * Ring Segment Size. This field defines the number of TRBs supported by the ring segment, Valid
	 * values for this field are 16 to 4096, i.e. an Event Ring segment shall contain at least 16 entries.
	 */
	WORD  RSSZ;
	WORD  RSV0;
	DWORD RSV1;
} XHCI_EVENT_RING_SEGMENT;
typedef struct _XHCI_DOORBELL
{
	/*
	DB Target – RW. Doorbell Target. This field defines the target of the doorbell reference. The
	table below defines the xHC notification that is generated by ringing the doorbell. Note that
	Doorbell Register 0 is dedicated to Command Ring and decodes this field differently than the
	other Doorbell Registers.

	Device Context Doorbells (1-255)
	Value Definition
	  0 Reserved
	  1 Control EP 0 Enqueue Pointer Update
	  2 EP 1 OUT Enqueue Pointer Update
	  3 EP 1 IN Enqueue Pointer Update
	  4 EP 2 OUT Enqueue Pointer Update
	  5 EP 2 IN Enqueue Pointer Update
	  … ...
	  30 EP 15 OUT Enqueue Pointer Update
	  31 EP 15 IN Enqueue Pointer Update
	  32:247 Reserved
	  248:255 Vendor Defined

	Host Controller Doorbell (0)
	Value Definition
	  0 Command Doorbell
	  1:247 Reserved
	  248:255 Vendor Defined

	This field returns ‘0’ when read and should be treated as “undefined” by software.
	When the Command Doorbell is written, the DB Stream ID field shall be cleared to ‘0’.
	*/
	DWORD TRGT:8;
	DWORD RSV0:8;
	/**
	 * DB Stream ID - RW. Doorbell Stream ID. If the endpoint of a Device Context Doorbell defines
	 * Streams, then this field shall be used to identify which Stream of the endpoint the doorbell
	 * reference is targeting. System software is responsible for ensuring that the value written to this
	 * field is valid.
	 *
	 * If the endpoint defines Streams (MaxPStreams > 0), then 0, 65535 (No Stream) and 65534
	 * (Prime) are reserved Stream ID values and shall not be written to this field.
	 *
	 * If the endpoint does not define Streams (MaxPStreams = 0) and a non-'0' value is written to this
	 * field, the doorbell reference shall be ignored.
	 *
	 * This field only applies to Device Context Doorbells and shall be cleared to ‘0’ for Host Controller
	 * Command Doorbells.
	 *
	 * This field returns ‘0’ when read.
	 */
	DWORD TASK:16;
} XHCI_DOORBELL;

typedef struct _XHCI_EXTENDED_CAPABILITY
{
	/**
	 * Capability ID – RO. This field identifies the xHCI Extended capability. Refer to Table 7-2 for a
	 * list of the valid xHCI extended capabilities.
	 */
	BYTE CAID;
	/**
	 * Next xHCI Extended Capability Pointer – RO. This field points to the xHC MMIO space offset of
	 * the next xHCI extended capability pointer. A value of 00h indicates the end of the extended
	 * capability list. A non-zero value in this register indicates a relative offset, in Dwords, from this
	 * Dword to the beginning of the next extended capability.
	 *
	 * For example, assuming an effective address of this data structure is 350h and assuming a
	 * pointer value of 068h, we can calculate the following effective address:
	 * 350h + (068h << 2) -> 350h + 1A0h -> 4F0h
	 */
	BYTE NEXT;
} XHCI_EXTENDED_CAPABILITY;
typedef struct _XHCI_PROTOCOL_SPEED_ID
{
	/**
	 * Protocol Speed ID Value (PSIV) – RO. If a device is attached that operates at the bit rate defined
	 * by this PSI Dword, then the value of this field shall be reported in the Port Speed field of
	 * PORTSC register (5.4.8) of a compatible port.
	 * Note, the PSIV value of ‘0’ is reserved and shall not be defined by a PSI
	 */
	DWORD PSIV:4;
	/**
	 * Protocol Speed ID Exponent (PSIE) – RO. This field defines the base 10 exponent times 3, that
	 * shall be applied to the Protocol Speed ID Mantissa when calculating the maximum bit rate
	 * represented by this PSI Dword.
	 *
	 * PSIE Value Bit Rate:
	 * - 0 Bits per second
	 * - 1 Kb/s
	 * - 2 Mb/s
	 * - 3 Gb/s
	 */
	DWORD PSIE:2;
	/**
	 * PSI Type (PLT) – RO. This field identifies whether the PSI Dword defines a symmetric or
	 * asymmetric bit rate, and if asymmetric, then this field also indicates if this Dword defines the
	 * receive or transmit bit rate.
	 *
	 * Note that the Asymmetric PSI Dwords shall be paired, i.e. an Rx immediately followed by a Tx,
	 * and both Dwords shall define the same value for the PSIV.
	 *
	 * PLT Value Bit Rate Note
	 * - 0 Symmetric Single PSI Dword
	 * - 1 Reserved
	 * - 2 Asymmetric Rx Paired with Asymmetric Tx PSI Dword
	 * - 3 Asymmetric Tx Immediately follows Rx Asymmetric PSI Dword
	 */
	DWORD PSIT:2;
	/**
	 * PSI Full-duplex (PFD) – RO. If this bit is ‘1’ the link is full-duplex (dual-simplex), and if ‘0’ the link
	 * is half-duplex (simplex).
	 */
	DWORD PSFD:1;
	DWORD RSV0:5;
	/**
	 * Link Protocol (LP) - RO. if xHCI Protocol Extended Capability:Major Revision = 03h, then this
	 * field identifies the link-level protocol supported by the ports associated with this PSI Dword.
	 * Refer to section 8.5.6.7 in the USB3 spec for more information. If xHCI Protocol Extended
	 * Capability:Major Revision = 02h, then this field shall be ‘0’, and the link protocol (LS, FS, or HS)
	 * depends on the reported link speed.
	 *
	 *  LP Value Protocol
	 *  - 0 SuperSpeed
	 *  - 1 SuperSpeedPlus
	 *  - 3-2 Reserved
	 */
	DWORD LINK:2;
	/**
	 * Protocol Speed ID Mantissa (PSIM) – RO. This field defines the mantissa that shall be applied to
	 * the PSIE when calculating the maximum bit rate represented by this PSI Dword.
	 */
	DWORD PSIM:16;
} XHCI_PROTOCOL_SPEED_ID;
typedef struct _XHCI_CAPABILITY_SUPPORTED_PROTOCOL
{
	XHCI_EXTENDED_CAPABILITY XECP;
	/**
	 * Minor Revision – RO. Minor Specification Release Number in Binary-Coded Decimal (i.e., version
	 * x.10 is 10h). This field identifies the minor release number component of the specification with
	 * which the xHC is compliant.
	 */
	BYTE  MINV;
	/**
	 * Major Revision – RO. Major Specification Release Number in Binary-Coded Decimal (i.e., version
	 * 3.x is 03h). This field identifies the major release number component of the specification with
	 * which the xHC is compliant.
	 */
	BYTE  MAJV;
	/**
	 * Name String – RO. This field is a mnemonic name string that references the specification with
	 * which the xHC is compliant. Four ASCII characters may be defined. Allowed characters are:
	 * alphanumeric, space, and underscore. Alpha characters are case sensitive. Refer to section 7.2.2
	 * for defined values.
	 */
	DWORD NAME;
	/**
	 * Compatible Port Offset – RO. This field specifies the starting Port Number of Root Hub Ports
	 * that support this protocol. Valid values are ‘1’ to MaxPorts.
	 */
	BYTE  CPOF;
	/**
	 * Compatible Port Count – RO. This field identifies the number of consecutive Root Hub Ports
	 * (starting at the Compatible Port Offset) that support this protocol. Valid values are 1 to
	 * MaxPorts.
	 */
	BYTE  CPCN;
	/**
	 * Protocol Defined. This field is reserved for protocol specific definitions. Refer to section
	 * 7.2.2.1.3.
	 */
	WORD  POTO:12;
	/**
	 * Protocol Speed ID Count (PSIC) – RO. This field indicates the number of Protocol Speed ID (PSI)
	 * Dwords that the xHCI Supported Protocol Capability data structure contains.
	 *
	 * If this field is non-zero, then all speeds supported by the protocol shall be defined using PSI
	 * Dwords, i.e. no implied Speed ID mappings apply.
	 *
	 * Refer to section 7.2.2 and its subsections for protocol specific requirements related to this field.
	 */
	WORD  PSIC:4;
	/**
	 * Protocol Slot Type – RO. This field specifies the Slot Type value which may be specified
	 * when allocating Device Slots that support this protocol. Valid values are ‘0’ to ‘31’. Refer to
	 * sections 4.6.3 and 7.2.2.1.4.
	 */
	DWORD PSTY:5;
	DWORD RSV0:27;
	/**
	 * Protocol Speed ID (PSI) Dwords immediately follow the Dword at offset 10h in
	 * an xHCI Supported Protocol Capability data structure. Table 7-10 defines the
	 * fields of a PSI Dword.
	 */
	XHCI_PROTOCOL_SPEED_ID PSIS[];
} XHCI_CAPABILITY_SUPPORTED_PROTOCOL;

#endif //SUPERNOVA_XHC_REGS_H
