//
// Created by MeiVi on 2026/04/08.
//

#pragma once

#define USB_REQ_GET_MAX_LUN 0xFE
#define USB_REQ_BULK_RESET  0xFF

#define CBW_SIGNATURE 0x43425355 // USBC
#define CSW_SIGNATURE 0x53425355 // USBS

#include <types.h>
#include <driver/xhci/xhci_device.h>

typedef struct _USB_MASS_STORAGE_BULK_CBW
{
	/**
	 * Signature that helps identify this data packet as a CBW. The signature field shall contain the value
	 * 43425355h (little endian), indicating a CBW.
	 */
	DWORD SIGN;
	/**
	 * A Command Block Tag sent by the host. The device shall echo the contents of this field back to the
	 * host in the dCSWTag field of the associated CSW. The dCSWTag positively associates a CSW with the
	 * corresponding CBW.
	 */
	DWORD TAGX;
	/**
	 * The number of bytes of data that the host expects to transfer on the Bulk-In or Bulk-Out endpoint (as
	 * indicated by the Direction bit) during the execution of this command. If this field is zero, the device and
	 * the host shall transfer no data between the CBW and the associated CSW, and the device shall ignore
	 * the value of the Direction bit in bmCBWFlags.
	 */
	DWORD DTRL;
	/**
	 * The bits of this field are defined as follows:
	 * - Bit 7 Direction - the device shall ignore this bit if the dCBWDataTransferLength field is
	 *   zero, otherwise:
	 *   - 0 = Data-Out from host to the device,
	 *   - 1 = Data-In from the device to the host.
	 * - Bit 6 Obsolete. The host shall set this bit to zero.
	 * - Bits 5..0 Reserved - the host shall set these bits to zero.
	 */
	BYTE  FLAG;
	/**
	 * The device Logical Unit Number (LUN) to which the command block is being sent. For devices that
	 * support multiple LUNs, the host shall place into this field the LUN to which this command block is
	 * addressed. Otherwise, the host shall set this field to zero.
	 */
	BYTE  CLUN;
	/**
	 * The valid length of the CBWCB in bytes. This defines the valid length of the command block. The
	 * only legal values are 1 through 16 (01h through 10h). All other values are reserved.
	 */
	BYTE  CLEN;
	/**
	 * The command block to be executed by the device. The device shall interpret the first bCBWCBLength
	 * bytes in this field as a command block as defined by the command set identified by bInterfaceSubClass.
	 *
	 * If the command set supported by the device uses command blocks of fewer than 16 (10h) bytes in
	 * length, the significant bytes shall be transferred first, beginning with the byte at offset 15 (Fh). The
	 * device shall ignore the content of the CBWCB field past the byte at offset (15 + bCBWCBLength - 1).
	 */
	BYTE  CMMD[16];
} USB_MASS_STORAGE_BULK_CBW;
typedef struct _USB_MASS_STORAGE_BULK_CSW
{
	/**
	 * Signature that helps identify this data packet as a CSW. The signature field shall contain the value
	 * 53425355h (little endian), indicating CSW.
	 */
	DWORD SIGN;
	// The device shall set this field to the value received in the dCBWTag of the associated CBW.
	DWORD TAGX;
	/**
	 * For Data-Out the device shall report in the dCSWDataResidue the difference between the amount of
	 * data expected as stated in the dCBWDataTransferLength, and the actual amount of data processed by
	 * the device. For Data-In the device shall report in the dCSWDataResidue the difference between the
	 * amount of data expected as stated in the dCBWDataTransferLength and the actual amount of relevant
	 * data sent by the device. The dCSWDataResidue shall not exceed the value sent in the
	 * dCBWDataTransferLength.
	 */
	DWORD DATA;
	/**
	 * bCSWStatus indicates the success or failure of the command. The device shall set this byte to zero if
	 * the command completed successfully. A non-zero value shall indicate a failure during command
	 * execution according to the following table:
	 * - 00h          | Command Passed ("good status")
	 * - 01h          | Command Failed
	 * - 02h          | Phase Error
	 * - 03h and 04h  | Reserved (Obsolete)
	 * - 05h to FFh   | Reserved
	 */
	BYTE  STAT;
} USB_MASS_STORAGE_BULK_CSW;

void xhci_usb_msc_bot(XHCI_USB_DEVICE *device);