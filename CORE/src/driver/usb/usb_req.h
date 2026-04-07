//
// Created by MeiVi on 2026/04/02.
//

#ifndef SUPERNOVAOS_USB_REQ_H
#define SUPERNOVAOS_USB_REQ_H

#include <types.h>

#define USB_REQ_GET_STATUS         0
#define USB_REQ_CLEAR_FEATURE      1
#define USB_REQ_SET_FEATURE        3
#define USB_REQ_SET_ADDRESS        5
#define USB_REQ_GET_DESCRIPTOR     6
#define USB_REQ_SET_DESCRIPTOR     7
#define USB_REQ_GET_CONFIGURATION  8
#define USB_REQ_SET_CONFIGURATION  9
#define USB_REQ_GET_INTERFACE      10
#define USB_REQ_SET_INTERFACE      11
#define USB_REQ_SYNCH_FRAME        12

/*
bmRequestType |bRequest         |wValue          |wIndex    |wLength    |Data
--------------------------------------------------------------------------------------
00000000B     |CLEAR_FEATURE    |Feature         |Zero      |Zero       |None
00000001B     |                 |Selector        |Interface |           |
00000010B     |                 |                |Endpoint  |           |
--------------------------------------------------------------------------------------
10000000B     |GET_CONFIGURATION|Zero            |Zero      |One        |Configuration
              |                 |                |          |           |Value
--------------------------------------------------------------------------------------
10000000B     |GET_DESCRIPTOR   |Descriptor      |Zero or   |Descriptor |Descriptor
              |                 |Type and        |Language  |Length     |
              |                 |Descriptor      |ID        |           |
              |                 |Index           |          |           |
--------------------------------------------------------------------------------------
10000001B     |GET_INTERFACE    |Zero            |Interface |One        |Alternate
              |                 |                |          |           |Interface
--------------------------------------------------------------------------------------
10000000B     |GET_STATUS       |Zero            |Zero      |Two        |Device,
10000001B     |                 |                |Interface |           |Interface, or
10000010B     |                 |                |Endpoint  |           |Endpoint
              |                 |                |          |           |Status
--------------------------------------------------------------------------------------
00000000B     |SET_ADDRESS      |Device          |Zero      |Zero       |None
              |                 |Address         |          |           |
--------------------------------------------------------------------------------------
00000000B     |SET_CONFIGURATION|Configuration   |Zero      |Zero       |None
              |                 |Value           |          |           |
--------------------------------------------------------------------------------------
00000000B     |SET_DESCRIPTOR   |Descriptor      |Zero or   |Descriptor |Descriptor
              |                 |Type and        |Language  |Length     |
              |                 |Descriptor      |Id        |           |
              |                 |Index           |          |           |
--------------------------------------------------------------------------------------
00000000B     |SET_FEATURE      |Feature         |Zero      |Zero       |Zero
00000001B     |                 |Selector        |Interface |           |
00000010B     |                 |                |Endpoint  |           |
--------------------------------------------------------------------------------------
00000001B     |SET_INTERFACE    |Alternate       |Interface |Zero       |None
              |                 |Setting         |          |           |
--------------------------------------------------------------------------------------
10000010B     |SYNCH_FRAME      |Zero            |Endpoint  |Zero       |Frame Number
*/

typedef struct _USB_DEVICE_SETUP_DATA
{
	/**
	 * D4...0: Recipient
	 * - 0 = Device
	 * - 1 = Interface
	 * - 2 = Endpoint
	 * - 3 = Other
	 * - 4...31 = Reserved
	 */
	DWORD RECP:5;
	/**
	 * D6...5: Type
	 * - 0 = Standard
	 * - 1 = Class
	 * - 2 = Vendor
	 * - 3 = Reserved
	 */
	DWORD RTYP:2;
	/**
	 * D7: Data transfer direction
	 * - 0 = Host-to-device
	 * - 1 = Device-to-host
	 */
	DWORD DIRE:1;
	// Specific request (refer to Table 9-3)
	DWORD REQU:8;
	// Word-sized field that varies according to request
	DWORD VALU:16;
	// Word-sized field that varies according to request; typically used to pass an index or offset
	DWORD INDX:16;
	// Number of bytes to transfer if there is a Data stage
	DWORD LENG:16;
} USB_DEVICE_SETUP_DATA;

#endif //SUPERNOVAOS_USB_REQ_H
