//
// Created by MeiVi on 2026/03/21.
//

#ifndef SUPERNOVA_XHC_TRB_H
#define SUPERNOVA_XHC_TRB_H

#include <types.h>

#define XHCI_TRB_TYPE_LINK               6
#define XHCI_TRB_TYPE_ENABLE_SLOT        9
#define XHCI_TRB_TYPE_DISABLE_SLOT       10
#define XHCI_TRB_TYPE_ADDRESS_DEVICE     11
#define XHCI_TRB_TYPE_COMMAND_COMPLETION 33
#define XHCI_TRB_TYPE_PORT_STATUS_CHANGE 34

#define XHCI_TRB_CTRL_CYCLE 1

/**
 * Indicates that the Completion Code field has not been updated by the
 * TRB producer.
 */
#define XHCI_CODE_INVALID                    0
// Indicates successful completion of the TRB operation.
#define XHCI_CODE_SUCCESS                    1
#define XHCI_CODE_DATA_BUFFER_ERROR          2
#define XHCI_CODE_BUBBLE_DETECTED_ERROR      3
#define XHCI_CODE_USB_TRANSACTION_ERROR      4
#define XHCI_CODE_TRB_ERROR                  5
#define XHCI_CODE_STALL_ERROR                6
#define XHCI_CODE_RESOURCE_ERROR             7
#define XHCI_CODE_BANDWIDTH_ERROR            8
#define XHCI_CODE_NO_SLOT_AVAILABLE_ERROR    9
#define XHCI_CODE_INVALID_STREAM_TYPE_ERROR  10
#define XHCI_CODE_SLOT_NOT_ENABLED_ERROR     11
#define XHCI_CODE_ENDPOINT_NOT_ENABLED_ERROR 12
#define XHCI_CODE_SHORT_PACKET               13
#define XHCI_CODE_UNDERRUN                   14
#define XHCI_CODE_RING_OVERRUN               15

typedef struct _XHCI_TRB_GENERIC
{
	QWORD DATA;
	DWORD STAT;
	DWORD CTRL;
} XHCI_TRB_GENERIC;
typedef struct _XHCI_TRB_NORMAL
{
	QWORD DATA;
	DWORD TRBL:17; // TRB Transfer Length
	DWORD TDSZ:5; // TD Size
	DWORD INTT:10; // Interrupter Target
	DWORD CYCL:1; // Cycle bit
	DWORD NEXT:1; // Evaluate Next TRB
	DWORD ISPK:1; // Interrupt-on Short Packet
	DWORD NOSN:1; // No Snoop
	DWORD CHAN:1; // Chain bit
	DWORD IONC:1; // Interrupt On Completion
	DWORD IMMD:1; // Immediate Data
	DWORD RSV0:2;
	DWORD BEIT:1; // Block Event Interrupt
	DWORD TYPE:6; // TRB Type
	DWORD RSV1:16;
} XHCI_TRB_NORMAL;


typedef struct _XHCI_TRB_LINK
{
	QWORD RING; // Ring Segment Pointer, should on a 16-byte boundry
	DWORD RSV0:22;
	DWORD INTT:10; // Interrupter Target
	DWORD CYCL:1; // Cycle bit
	DWORD CYCT:1; // Cycle Toggle
	DWORD RSV1:2;
	DWORD CHAN:1; // Chain bit
	DWORD IONC:1; // Interrupt On Completion
	DWORD RSV2:4;
	DWORD TYPE:6; // TRB Type
	DWORD RSV3:16;
} XHCI_TRB_LINK;
typedef struct _XHCI_TRB_ENABLE_SLOT
{
	QWORD RSV0;
	DWORD RSV1;
	/**
	 * Cycle bit (C). This bit is used to mark the Enqueue Pointer of a Command Ring.
	 */
	DWORD CYCL:1;
	DWORD RSV2:9;
	/**
	 * TRB Type. This field identifies the type of the TRB. Refer to Table 6-91 for the definition of the
	 * Enable Slot Command TRB type ID.
	 */
	DWORD TYPE:6;
	/**
	 * Slot Type. This field identifies the type of Slot that will be enabled by this command. Refer to
	 * Table 7-9 for more information on the usage of Slot Type.
	 */
	DWORD SLOT:5;
	DWORD RSV3:11;
} XHCI_TRB_ENABLE_SLOT;
typedef struct _XHCI_TRB_DISABLE_SLOT
{
	QWORD RSV0;
	DWORD RSV1;
	/**
	 * Cycle bit (C). This bit is used to mark the Enqueue Pointer of a Command Ring.
	 */
	DWORD CYCL:1;
	DWORD RSV2:9;
	/**
	 * TRB Type. This field identifies the type of the TRB. Refer to Table 6-91 for the definition of the
	 * Disable Slot Command TRB type ID.
	 */
	DWORD TYPE:6;
	DWORD RSV3:8;
	// Slot ID. The ID of the Device Slot to disable.
	DWORD SLOT:8;
} XHCI_TRB_DISABLE_SLOT;
typedef struct _XHCI_TRB_ADDRESS_DEVICE
{
	/**
	 * Input Context Pointer Hi and Lo. This field represents the high order bits of the 64-bit base
	 * address of the Input Context data structure associated with this command. Refer to section 6.2.5
	 * for more information on the Input Context data structure.
	 *
	 * The memory structure referenced by this physical memory pointer shall be aligned on a 16-byte
	 * address boundary.
	 */
	QWORD CTXT;
	DWORD RSV0;
	// Cycle bit (C). This bit is used to mark the Enqueue Pointer of a Command Ring.
	DWORD CYCL:1;
	DWORD RSV1:8;
	/**
	 * Block Set Address Request (BSR). When this flag is set to ‘0’ the Address Device Command shall
	 * generate a USB SET_ADDRESS request to the device. When this flag is set to ‘1’ the Address
	 * Device Command shall not generate a USB SET_ADDRESS request. Refer to section 4.6.5 for
	 * more information on the use of this flag.
	 */
	DWORD BSAR:1;
	/**
	 * TRB Type. This field identifies the type of the TRB. Refer to Table 6-91 for the definition of the
	 * Address Device Command TRB type ID.
	 */
	DWORD TYPE:6;
	DWORD RSV2:8;
	// Slot ID. The ID of the Device Slot that is the target of this command.
	DWORD SLOT:8;
} XHCI_TRB_ADDRESS_DEVICE;
typedef struct _XHCI_TRB_COMMAND_COMPLETION
{
	/**
	 * Command TRB Pointer Hi and Lo. This field represents the high order bits of the 64-bit address
	 * of the Command TRB that generated this event. Note that this field is not valid for some
	 * Completion Code values. Refer to Table 6-90 for specific cases.
	 *
	 * The memory structure referenced by this physical memory pointer shall be aligned on a 16-byte
	 * address boundary.
	 */
	QWORD CMMD;
	/**
	 * Command Completion Parameter. This field may optionally be set by a command. Refer to
	 * section 4.6.6.1 for specific usage. If a command does not utilize this field it shall be treated as
	 * RsvdZ.
	 */
	DWORD PARM:24;
	/**
	 * Completion Code. This field encodes the completion status of the command that generated the
	 * event. Refer to the respective command definition for a list of the possible Completion Codes
	 * associated with the command. Refer to section 6.4.5 for an enumerated list of possible error
	 * conditions.
	 */
	DWORD CCOD:8;
	// Cycle bit (C). This bit is used to mark the Dequeue Pointer of an Event Ring.
	WORD  CYCL:1;
	WORD  RSV0:9;
	/**
	 * TRB Type. This field identifies the type of the TRB. Refer to Table 6-91 for the definition of the
	 * Command Completion Event TRB type ID.
	 */
	WORD  TYPE:6;
	/**
	 * VF ID. The ID of the Virtual Function that generated the event. Note that this field is valid only if
	 * Virtual Functions are enabled. If they are not enabled this field shall be cleared to ‘0’.
	 */
	BYTE  VFID;
	/**
	 * Slot ID. The Slot ID field shall be updated by the xHC to reflect the slot associated with the
	 * command that generated the event, with the following exceptions:
	 *
	 * - The Slot ID shall be cleared to ‘0’ for No Op, Set Latency Tolerance Value, Get Port Bandwidth,
	 * and Force Event Commands.
	 *
	 * - The Slot ID shall be set to the ID of the newly allocated Device Slot for the Enable Slot
	 * Command.
	 *
	 * - The value of Slot ID shall be vendor defined when generated by a vendor defined command.
	 * 
	 * This value is used as an index in the Device Context Base Address Array to select the Device
	 * Context of the source device. If this Event is due to a Host Controller Command, then this field
	 * shall be cleared to ‘0’.
	 */
	BYTE  SLID;
} XHCI_TRB_COMMAND_COMPLETION;
typedef struct _XHCI_TRB_PORT_STATUS_CHANGE
{
	WORD  RSV0;
	BYTE  RSV1;
	// Port ID. The Port Number of the Root Hub Port that generated this event.
	BYTE  PRID;
	DWORD RSV2;
	WORD  RSV3;
	BYTE  RSV4;
	/**
	 * Completion Code. This field encodes the completion status that can be identified by a TRB. The
	 * Completion Code field shall be set to Success.
	 */
	BYTE  CCOD;
	// Cycle bit (C). This bit is used to mark the Dequeue Pointer of an Event Ring.
	WORD  CYCL:1;
	WORD  RSV5:9;
	/**
	 * TRB Type. This field identifies the type of the TRB. Refer to Table 6-91 for the definition of the
	 * Port Status Change Event TRB type ID.
	 */
	WORD  TYPE:6;
	WORD  RSV6;
} XHCI_TRB_PORT_STATUS_CHANGE;

#endif //SUPERNOVA_XHC_TRB_H
