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
	/**
	 * Number of Ports (MaxPorts). This field specifies the maximum Port Number value, i.e. the
	 * highest numbered Port Register Set that are addressable in the Operational Register Space
	 * (refer to Table 5-18). Valid values are in the range of 1h to FFh.
	 *
	 * The value in this field shall reflect the maximum Port Number value assigned by an xHCI
	 * Supported Protocol Capability, described in section 7.2. Software shall refer to these capabilities
	 * to identify whether a specific Port Number is valid, and the protocol supported by the
	 * associated Port Register Set.
	 */
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
	/**
	 * Context Size (CSZ). If this bit is set to ‘1’, then the xHC uses 64 byte Context data structures. If
	 * this bit is cleared to ‘0’, then the xHC uses 32 byte Context data structures.
	 * Note: This flag does not apply to Stream Contexts.
	 */
	DWORD CSZE:1;
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
typedef struct _XHCI_PORT_SPACE
{
	union
	{
		DWORD PTSC;
		struct
		{
			/**
			 * Current Connect Status (CCS) – ROS. Default = ‘0’. ‘1’ = A device is connected to the port. ‘0’ =
			 * A device is not connected. This value reflects the current state of the port, and may not
			 * correspond directly to the event that caused the Connect Status Change (CSC) bit to be set to ‘1’.
			 * Refer to sections 4.19.3 and 4.19.4 for more details on the Connect Status Change (CSC)
			 * assertion conditions.
			 *
			 * This flag is ‘0’ if PP is ‘0’.
			 */
			DWORD CCSS:1;
			/**
			 * Port Enabled/Disabled (PED) – RW1CS. Default = ‘0’. ‘1’ = Enabled. ‘0’ = Disabled.
			 * Ports may only be enabled by the xHC. Software cannot enable a port by writing a ‘1’ to this flag.
			 * A port may be disabled by software writing a ‘1’ to this flag.
			 *
			 * This flag shall automatically be cleared to ‘0’ by a disconnect event or other fault condition.
			 * Note that the bit status does not change until the port state actually changes. There may be a
			 * delay in disabling or enabling a port due to other host controller or bus events.
			 *
			 * When the port is disabled (PED = ‘0’) downstream propagation of data is blocked on this port,
			 * except for reset.
			 *
			 * For USB2 protocol ports:
			 *
			 * When the port is in the Disabled state, software shall reset the port (PR = ‘1’) to transition PED to
			 * ‘1’ and the port to the Enabled state.
			 *
			 * For USB3 protocol ports:
			 *
			 * When the port is in the Polling state (after detecting an attach), the port shall automatically
			 * transition to the Enabled state and set PED to ‘1’ upon the completion of successful link training.
			 * When the port is in the Disabled state, software shall write a ‘5’ (RxDetect) to the PLS field to
			 * transition the port to the Disconnected state. Refer to section 4.19.1.2.
			 *
			 * PED shall automatically be cleared to ‘0’ when PR is set to ‘1’, and set to ‘1’ when PR transitions
			 * from ‘1’ to ‘0’ after a successful reset. Refer to Port Reset (PR) bit for more information on how
			 * the PED bit is managed.
			 *
			 * Note that when software writes this bit to a ‘1’, it shall also write a ‘0’ to the PR bit82
			 * .
			 *
			 * This flag is ‘0’ if PP is ‘0’.
			 */
			DWORD POEN:1;
			DWORD RSV0:1;
			/**
			 * Over-current Active (OCA) – RO. Default = ‘0’. ‘1’ = This port currently has an over-current
			 * condition. ‘0’ = This port does not have an over-current condition. This bit shall automatically
			 * transition from a ‘1’ to a ‘0’ when the over-current condition is removed.
			 */
			DWORD OCAC:1;
			/**
			 * Port Reset (PR) – RW1S. Default = ‘0’. ‘1’ = Port Reset signaling is asserted. ‘0’ = Port is not in
			 * Reset. When software writes a ‘1’ to this bit generating a ‘0’ to ‘1’ transition, the bus reset
			 * sequence is initiated83; USB2 protocol ports shall execute the bus reset sequence as defined in
			 * the USB2 Spec. USB3 protocol ports shall execute the Hot Reset sequence as defined in the
			 * USB3 Spec. PR remains set until reset signaling is completed by the root hub.
			 *
			 * Note that software shall write a ‘1’ to this flag to transition a USB2 port from the Polling state to
			 * the Enabled state. Refer to sections 4.15.2.3 and 4.19.1.1.
			 *
			 * This flag is ‘0’ if PP is ‘0’.
			 */
			DWORD PRST:1;
			/**
			 * Port Link State (PLS) – RWS. Default = RxDetect (‘5’). This field is used to power manage the port
			 * and reflects its current link state.
			 *
			 * When the port is in the Enabled state, system software may set the link U state by writing this
			 * field. System software may also write this field to force a Disabled to Disconnected state
			 * transition of the port.
			 *
			 * Write Value Description
			 * - 0 The link shall transition to a U0 state from any of the U states.
			 * - 2 USB2 protocol ports only. The link should transition to the U2 State.
			 * - 3 The link shall transition to a U3 state from the U0 state. This action
			 * selectively suspends the device connected to this port. While the Port
			 * Link State = U3, the hub does not propagate downstream-directed
			 * traffic to this port, but the hub shall respond to resume signaling from
			 * the port.
			 * - 5 USB3 protocol ports only. If the port is in the Disabled state (PLS =
			 * Disabled, PP = 1), then the link shall transition to a RxDetect state and
			 * the port shall transition to the Disconnected state, else ignored.
			 * - 10 USB3 protocol ports only. Shall enable a link transition to the
			 * Compliance state, i.e. CTE = ‘1’. Refer to section 4.19.1.2.4.1 for more
			 * information.
			 * - 1,4,6-9,11-14 Ignored.
			 * - 15 USB2 protocol ports only. If the port is in the U3 state (PLS = U3), then
			 * the link shall remain in the U3 state and the port shall transition to the
			 * Resume substate, else ignored. Refer to section 4.15.2 for more
			 * information.
			 *
			 * Note: The Port Link State Write Strobe (LWS) shall also be set to ‘1’ to write this
			 * field.
			 *
			 * For USB2 protocol ports: Writing a value of '2' to this field shall request LPM, asserting L1
			 * signaling on the USB2 bus. Software may read this field to determine if the transition to the U2
			 * state was successful. Writing a value of '0' shall deassert L1 signaling on the USB. Writing a value
			 * of '1' shall have no effect. The U1 state shall never be reported by a USB2 protocol port.
			 *
			 * Read Value Meaning
			 * - 0 Link is in the U0 State
			 * - 1 Link is in the U1 State
			 * - 2 Link is in the U2 State
			 * - 3 Link is in the U3 State (Device Suspended)
			 * - 4 Link is in the Disabled State86
			 * - 5 Link is in the RxDetect State87
			 * - 6 Link is in the Inactive State88
			 * - 7 Link is in the Polling State
			 * - 8 Link is in the Recovery State
			 * - 9 Link is in the Hot Reset State
			 * - 10 Link is in the Compliance Mode State
			 * - 11 Link is in the Test Mode89 State
			 * - 12-14 Reserved
			 * - 15 Link is in the Resume State90
			 *
			 * This field is undefined if PP = ‘0’.
			 *
			 * Note: Transitions between different states are not reflected until the transition is complete. Refer
			 * to section 4.19 for PLS transition conditions.
			 */
			DWORD PLST:4;
			/**
			 * Port Power (PP) – RWS. Default = ‘1’. This flag reflects a port's logical, power control state.
			 * Because host controllers can implement different methods of port power switching, this flag may
			 * or may not represent whether (VBus) power is actually applied to the port. When PP equals a '0'
			 * the port is nonfunctional and shall not report attaches, detaches, or Port Link State (PLS)
			 * changes. However, the port shall report over-current conditions when PP = ‘0’ if PPC = ‘0’. After
			 * modifying PP, software shall read PP and confirm that it is reached its target state before
			 * modifying it again91, undefined behavior may occur if this procedure is not followed.
			 * 0 = This port is in the Powered-off state.
			 * 1 = This port is not in the Powered-off state.
			 * If the Port Power Control (PPC) flag in the HCCPARAMS1 register is '1', then xHC has port power
			 * control switches and this bit represents the current setting of the switch ('0' = off, '1' = on).
			 * If the Port Power Control (PPC) flag in the HCCPARAMS1 register is '0', then xHC does not have
			 * port power control switches and each port is hard wired to power, and not affected by this bit.
			 * When an over-current condition is detected on a powered port, the xHC shall transition the PP
			 * bit in each affected port from a ‘1’ to ‘0’ (removing power from the port).
			 * Note: If this is an SSIC Port, then the DSP Disconnect process is initiated by '1' to '0' transition of
			 * PP. After an SSIC USP disconnect process, the port may be disabled by setting PED = 1. As noted,
			 * the SSIC spec does not define a mechanism for the USP to request DSP to be re-enabled for a
			 * subsequent re-connect. If PED is set to 1 without a prior negotiated disconnect with the USP,
			 * subsequent re-enabling of the port requires DSP to issue a WPR to bring USP back to Rx.Detect.
			 * Refer to section 5.1.2 in the SSIC Spec for more information.
			 * Refer to section 4.19.4 for more information.
			 */
			DWORD PPWR:1;
			/**
			 * Port Speed (Port Speed) – ROS. Default = ‘0’. This field identifies the speed of the connected
			 * USB Device. This field is only relevant if a device is connected (CCS = ‘1’) in all other cases this
			 * field shall indicate Undefined Speed. Refer to section 4.19.3.
			 *
			 * Value Meaning
			 * - 0 Undefined Speed
			 * - 1 - 15 Protocol Speed ID (PSI), refer to section 7.2.1 for the definition of PSIV
			 * field in the PSI Dword
			 *
			 * Note: This field is invalid on a USB2 protocol port until after the port is reset.
			 */
			DWORD PSPD:4;
			/**
			 * Port Indicator Control (PIC) – RWS. Default = 0. Writing to these bits has no effect if the Port
			 * Indicators (PIND) bit in the HCCPARAMS1 register is a ‘0’. If PIND bit is a ‘1’, then the bit
			 * encodings are:
			 *
			 * Value Meaning
			 * - 0 Port indicators are off
			 * - 1 Amber
			 * - 2 Green
			 * - 3 Undefined
			 *
			 * Refer to the USB2 Specification section 11.5.3 for a description on how these bits shall be used.
			 *
			 * This field is ‘0’ if PP is ‘0’.
			 */
			DWORD PICN:2;
			/**
			 * Port Link State Write Strobe (LWS) – RW. Default = ‘0’. When this bit is set to ‘1’ on a write
			 * reference to this register, this flag enables writes to the PLS field. When ‘0’, write data in PLS field
			 * is ignored. Reads to this bit return ‘0’.
			 */
			DWORD LSWS:1;
			/**
			 * Connect Status Change (CSC) – RW1CS. Default = ‘0’. ‘1’ = Change in CCS. ‘0’ = No change. This
			 * flag indicates a change has occurred in the port’s Current Connect Status (CCS) or Cold Attach
			 * Status (CAS) bits. Note that this flag shall not be set if the CCS transition was due to software
			 * setting PP to ‘0’, or the CAS transition was due to software setting WPR to ‘1’. The xHC sets this
			 * bit to ‘1’ for all changes to the port device connect status92, even if system software has not
			 * cleared an existing Connect Status Change. For example, the insertion status changes twice
			 * before system software has cleared the changed condition, root hub hardware will be “setting”
			 * an already-set bit (i.e., the bit will remain ‘1’). Software shall clear this bit by writing a ‘1’ to it.
			 * Refer to section 4.19.2 for more information on change bit usage.
			 */
			DWORD CSCH:1;
			/**
			 * Port Enabled/Disabled Change (PEC) – RW1CS. Default = ‘0’. ‘1’ = change in PED. ‘0’ = No
			 * change. Note that this flag shall not be set if the PED transition was due to software setting PP to
			 * ‘0’. Software shall clear this bit by writing a ‘1’ to it. Refer to section 4.19.2 for more information
			 * on change bit usage.
			 *
			 * For a USB2 protocol port, this bit shall be set to ‘1’ only when the port is disabled due to the
			 * appropriate conditions existing at the EOF2 point (refer to section 11.8.1 of the USB2
			 * Specification for the definition of a Port Error).
			 *
			 * For a USB3 protocol port, this bit shall never be set to ‘1’.
			 */
			DWORD PECH:1;
			/**
			 * Warm Port Reset Change (WRC) – RW1CS/RsvdZ. Default = ‘0’. This bit is set when Warm Reset
			 * processing on this port completes. ‘0’ = No change. ‘1’ = Warm Reset complete. Note that this
			 * flag shall not be set to ‘1’ if the Warm Reset processing was forced to terminate due to software
			 * clearing PP or PED to '0'. Software shall clear this bit by writing a '1' to it. Refer to section 4.19.5.1.
			 * Refer to section 4.19.2 for more information on change bit usage.
			 *
			 * This bit only applies to USB3 protocol ports. For USB2 protocol ports it shall be RsvdZ.
			 */
			DWORD WRCH:1;
			/**
			 * Over-current Change (OCC) – RW1CS. Default = ‘0’. This bit shall be set to a ‘1’ when there is a ‘0’
			 * to ‘1’ or ‘1’ to ‘0’ transition of Over-current Active (OCA). Software shall clear this bit by writing a
			 * ‘1’ to it. Refer to section 4.19.2 for more information on change bit usage
			 */
			DWORD OCCH:1;
			/**
			 * Port Reset Change (PRC) – RW1CS. Default = ‘0’. This flag is set to ‘1’ due to a '1' to '0' transition
			 * of Port Reset (PR). e.g. when any reset processing (Warm or Hot) on this port is complete. Note
			 * that this flag shall not be set to ‘1’ if the reset processing was forced to terminate due to software
			 * clearing PP or PED to '0'. ‘0’ = No change. ‘1’ = Reset complete. Software shall clear this bit by
			 * writing a '1' to it. Refer to section 4.19.5. Refer to section 4.19.2 for more information on change
			 * bit usage.
			 */
			DWORD PRCH:1;
			/**
			 * Port Link State Change (PLC) – RW1CS. Default = ‘0’. This flag is set to ‘1’ due to the following
			 * PLS transitions:
			 *
			 * Transition Condition
			 * - U3 -> Resume Wakeup signaling from a device
			 * - Resume -> Recovery -> U0 Device Resume complete (USB3 protocol ports
			 * only)
			 * - Resume -> U0 Device Resume complete (USB2 protocol ports
			 * only)
			 * - U3 -> Recovery -> U0 Software Resume complete (USB3 protocol ports
			 * only)
			 * - U3 -> U0 Software Resume complete (USB2 protocol ports
			 * only)
			 * - U2 -> U0 L1 Resume complete (USB2 protocol ports only)93
			 * - U0 -> U0 L1 Entry Reject (USB2 protocol ports only)93
			 * - Any state -> Inactive Error (USB3 protocol ports only).
			 * Note: PLC is asserted only on the first LTSSM
			 * SS.Inactive.Disconnect.Detect to SS.Inactive.Quiet
			 * substate transition after entering the SS.Inactive
			 * state94.
			 * - Any State -> U3 U3 Entry complete. Note: PLC is asserted only if
			 * U3E = ‘1’95.
			 *
			 * Note that this flag shall not be set if the PLS transition was due to software
			 * setting PP to ‘0’. Refer to section 4.23.5 for more information. '0' = No
			 * change. '1' = Link Status Changed. Software shall clear this bit by
			 * writing a '1' to it. Refer to “PLC Condition:” references in section 4.19.1
			 * for the specific port state transitions that set this flag. Refer to section
			 * 4.19.2 for more information on change bit usage.
			 */
			DWORD PLCH:1;
			/**
			 * Port Config Error Change (CEC) – RW1CS/RsvdZ. Default = ‘0’. This flag indicates that the port
			 * failed to configure its link partner. 0 = No change. 1 = Port Config Error detected. Software shall
			 * clear this bit by writing a '1' to it. Refer to section 4.19.2 for more information on change bit
			 * usage.
			 *
			 * Note: This flag is valid only for USB3 protocol ports. For USB2 protocol ports this bit shall be
			 * RsvdZ.
			 */
			DWORD CECH:1;
			/**
			 * Cold Attach Status (CAS) – RO. Default = ‘0’. ‘1’ = Far-end Receiver Terminations were detected
			 * in the Disconnected state and the Root Hub Port State Machine was unable to advance to the
			 * Enabled state. Refer to sections 4.19.8 for more details on the Cold Attach Status (CAS) assertion
			 * conditions. Software shall clear this bit by writing a '1' to WPR or the xHC shall clear this bit if CCS
			 * transitions to ‘1’.
			 *
			 * This flag is ‘0’ if PP is ‘0’ or for USB2 protocol ports.
			 */
			DWORD CAST:1;
			/**
			 * Wake on Connect Enable (WCE) – RWS. Default = ‘0’. Writing this bit to a ‘1’ enables the port to
			 * be sensitive to device connects as system wake-up events96. Refer to section 4.15 for operational
			 * model.
			 */
			DWORD WCEN:1;
			/**
			 * Wake on Disconnect Enable (WDE) – RWS. Default = ‘0’. Writing this bit to a ‘1’ enables the port
			 * to be sensitive to device disconnects as system wake-up events96. Refer to section 4.15 for
			 * operational model.
			 */
			DWORD WDEN:1;
			/**
			 * Wake on Over-current Enable (WOE) – RWS. Default = ‘0’. Writing this bit to a ‘1’ enables the
			 * port to be sensitive to over-current conditions as system wake-up events96. Refer to section 4.15
			 * for operational model.
			 */
			DWORD WOEN:1;
			DWORD RSV1:2;
			/**
			 * Device Removable97 (DR) - RO. This flag indicates if this port has a removable device attached.
			 * ‘1’ = Device is non-removable. ‘0’ = Device is removable.
			 */
			DWORD RMOV:1;
			/**
			 * Warm Port Reset (WPR) – RW1S/RsvdZ. Default = ‘0’. When software writes a ‘1’ to this bit, the
			 * Warm Reset sequence as defined in the USB3 Specification is initiated and the PR flag is set to ‘1’.
			 * Once initiated, the PR, PRC, and WRC flags shall reflect the progress of the Warm Reset
			 * sequence. This flag shall always return ‘0’ when read. Refer to section 4.19.5.1.
			 *
			 * This flag only applies to USB3 protocol ports. For USB2 protocol ports it shall be RsvdZ.
			 */
			DWORD WRST:1;
		};
	};

	/**
	 * U1 Timeout – RWS. Default = ‘0’. Timeout value for U1 inactivity timer. If equal to FFh, the port
	 * is disabled from initiating U1 entry. This field shall be set to ‘0’ by the assertion of PR to ‘1’. Refer
	 * to section 4.19.4.1 for more information on U1 Timeout operation. The following are
	 * permissible values:
	 * Value Description
	 * - 00h Zero (default)
	 * - 01h 1 µs.
	 * - 02h 2 µs.
	 * - …
	 * - 7Fh 127 µs.
	 * - 80h–FEh Reserved
	 * - FFh Infinite
	 */
	BYTE  U1TO;
	/**
	 * U2 Timeout – RWS. Default = ‘0’. Timeout value for U2 inactivity timer. If equal to FFh, the port
	 * is disabled from initiating U2 entry. This field shall be set to ‘0’ by the assertion of PR to ‘1’. Refer
	 * to section 4.19.4.1 for more information on U2 Timeout operation. The following are
	 * permissible values:
	 * Value Description
	 * - 00h Zero (default)
	 * - 01h 256 µs
	 * - 02h 512 µs
	 * - …
	 * - FEh 65,024 ms
	 * - FFh Infinite
	 *
	 * A U2 Inactivity Timeout LMP shall be sent by the xHC to the device connected on this port when
	 * this field is written. Refer to Sections 8.4.3 and 10.4.2.10 of the USB3 specification for more
	 * details.
	 */
	BYTE  U2TO;
	/**
	 * Force Link PM Accept (FLA) - RW. Default = ‘0’. When this bit is set to ‘1’, the port shall generate
	 * a Set Link Function LMP with the Force_LinkPM_Accept bit asserted (‘1’). When this bit is cleared
	 * to ‘0’, the port shall generate a Set Link Function LMP with the Force_LinkPM_Accept bit deasserted (‘0’).
	 * This flag shall be set to ‘0’ by the assertion of PR to ‘1’ or when CCS = transitions from ‘0’ to ‘1’.
	 *
	 * Writes to this flag have no effect if PP = ‘0’.
	 *
	 * The Set Link Function LMP is sent by the xHC to the device connected on this port when this bit
	 * transitions from ‘0’ to ‘1’ or ‘1’ to ‘0’. Refer to Sections 8.4.2 and 10.14.2.2 of the USB3
	 * specification for more details.
	 *
	 * Improper use of the SS Force_LinkPM_Accept functionality can impact the performance of the
	 * link significantly. This bit shall only be used for compliance and testing purposes. Software shall
	 * ensure that there are no pending packets at the link level before setting this bit.
	 *
	 * This flag is ‘0’ if PP is ‘0’.
	 */
	WORD  FLPA:1;
	WORD  RSV2:15;
	/**
	 * Link Error Count – RW. Default = ‘0’. This field returns the number of link errors detected by the
	 * port. This value shall be reset to ‘0’ by the assertion of a Chip Hardware Reset, HCRST, when PR
	 * transitions from ‘1’ to ‘0’, or when reset by software by writing 0 to it. This register will increment
	 * by one each time a port transitions from U0 to Recovery to recover an error event and will
	 * saturate at max.
	 */
	WORD  LERR;
	/**
	 * Rx Lane Count (RLC) - RO. Default = '0'. This field that identifies the number of Receive Lanes
	 * negotiated by the port. This is a "zero-based" value, where 0 to 15 represents Lane Counts of 1
	 * to 16, respectively. This value is valid only when CCS = '1'. RLC shall equal '0' for a simplex
	 * Sublink. Refer to section 7.2.1 for more information.
	 */
	BYTE  RXLC:4;
	/**
	 * Tx Lane Count (TLC) - RO. Default = '0'. This field that identifies the number of Transmit Lanes
	 * negotiated by the port. This is a "zero-based" value, where 0 to 15 represents Lane Counts of 1
	 * to 16, respectively. This value is valid only when CCS = '1'. TLC shall equal '0' for a simplex
	 * Sublink. Refer to section 7.2.1 for more information.
	 */
	BYTE  TXLC:4;
	BYTE  RSV3;
	/**
	 * Link Soft Error Count – RW. Default = ‘0’. This field returns the number of link errors detected
	 * by the port. This value shall be reset to ‘0’ by the assertion of a Chip Hardware Reset, HCRST,
	 * when PR transitions from ‘1’ to ‘0’, or when reset by software by writing 0 to it. This register will
	 * saturate at max and will increment by one for all the conditions listed in section 7.3.2.2 (Soft
	 * Error Count) of the USB3.2 Specification.
	 */
	WORD  LSEC;
	WORD  RSV4;
} XHCI_PORT_SPACE;
typedef struct _XHCI_OPERATIONAL_SPACE
{
	union
	{
		DWORD UCMD;
		struct
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
		};
	};
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
	union
	{
		/**
		 * The Command Ring Control Register provides Command Ring control and status
		 * capabilities, and identifies the address and Cycle bit state of the Command Ring
		 * Dequeue Pointer.
		 */
		QWORD CRCR;
		struct
		{
			/**
			 * Ring Cycle State (RCS) - RW. This bit identifies the value of the xHC Consumer Cycle State (CCS)
			 * flag for the TRB referenced by the Command Ring Pointer. Refer to section 4.9.3 for more
			 * information.
			 *
			 * Writes to this flag are ignored if Command Ring Running (CRR) is ‘1’.
			 *
			 * If the CRCR is written while the Command Ring is stopped (CRR = ‘0’), then the value of this flag
			 * shall be used to fetch the first Command TRB the next time the Host Controller Doorbell register
			 * is written with the DB Reason field set to Host Controller Command.
			 *
			 * If the CRCR is not written while the Command Ring is stopped (CRR = ‘0’), then the Command
			 * Ring shall begin fetching Command TRBs using the current value of the internal Command Ring
			 * CCS flag.
			 *
			 * Reading this flag always returns ‘0’.
			 */
			QWORD CRCS:1;
			/**
			 * Command Stop (CS) - RW1S. Default = ‘0’. Writing a ‘1’ to this bit shall stop the operation of the
			 * Command Ring after the completion of the currently executing command, and generate a
			 * Command Completion Event with the Completion Code set to Command Ring Stopped and the
			 * Command TRB Pointer set to the current value of the Command Ring Dequeue Pointer. Refer to
			 * section 4.6.1.1 for more information on stopping a command.
			 *
			 * The next write to the Host Controller Doorbell with DB Reason field set to Host Controller
			 * Command shall restart the Command Ring operation.
			 *
			 * Writes to this flag are ignored by the xHC if Command Ring Running (CRR) = ‘0’.
			 * Reading this bit shall always return ‘0’.
			 */
			QWORD CSTP:1;
			/**
			 * Command Abort (CA) - RW1S. Default = ‘0’. Writing a ‘1’ to this bit shall immediately terminate
			 * the currently executing command, stop the Command Ring, and generate a Command
			 * Completion Event with the Completion Code set to Command Ring Stopped. Refer to section
			 * 4.6.1.2 for more information on aborting a command.
			 *
			 * The next write to the Host Controller Doorbell with DB Reason field set to Host Controller
			 * Command shall restart the Command Ring operation.
			 *
			 * Writes to this flag are ignored by the xHC if Command Ring Running (CRR) = ‘0’.
			 * Reading this bit always returns ‘0’.
			 */
			QWORD CABR:1;
			/**
			 * Command Ring Running (CRR) - RO. Default = 0. This flag is set to ‘1’ if the Run/Stop (R/S) bit is
			 * ‘1’ and the Host Controller Doorbell register is written with the DB Reason field set to Host
			 * Controller Command. It is cleared to ‘0’ when the Command Ring is “stopped” after writing a ‘1’
			 * to the Command Stop (CS) or Command Abort (CA) flags, or if the R/S bit is cleared to ‘0’.
			 */
			QWORD CRUN:1;
			QWORD RSVA:2;
			/**
			 * Command Ring Pointer - RW. Default = ‘0’. This field defines high order bits of the initial value
			 * of the 64-bit Command Ring Dequeue Pointer.
			 *
			 * Writes to this field are ignored when Command Ring Running (CRR) = ‘1’.
			 *
			 * If the CRCR is written while the Command Ring is stopped (CRR = ‘0’), the value of this field shall
			 * be used to fetch the first Command TRB the next time the Host Controller Doorbell register is
			 * written with the DB Reason field set to Host Controller Command.
			 *
			 * If the CRCR is not written while the Command Ring is stopped (CRR = ‘0’) then the Command
			 * Ring shall begin fetching Command TRBs at the current value of the internal xHC Command
			 * Ring Dequeue Pointer.
			 *
			 * Reading this field always returns ‘0’.
			 */
			QWORD CRNG:58;
		};
	};
	DWORD RSV7[4];
	QWORD CBAA;
	//DWORD configure;
	DWORD MDSE:8; // Max Device Slots Enabled
	DWORD ENU3:1; // Enable U3 Entry
	DWORD ENCI:1; // Enable Configuration Information
	DWORD RSV8:22;
	DWORD RSV9[0xF1];
	XHCI_PORT_SPACE PORT[];
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
