//
// Created by MeiVi on 2026/04/02.
//

#ifndef SUPERNOVAOS_USB_REQ_H
#define SUPERNOVAOS_USB_REQ_H

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

#define USB_DESC_DEVICE                    0x01
#define USB_DESC_CONFIGURATION             0x02
#define USB_DESC_STRING                    0x03
#define USB_DESC_INTERFACE                 0x04
#define USB_DESC_ENDPOINT                  0x05
#define USB_DESC_DEVICE_QUALIFIER          0x06
#define USB_DESC_OTHER_SPEED_CONFIGURATION 0x07
#define USB_DESC_INTERFACE_POWER           0x08

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

#endif //SUPERNOVAOS_USB_REQ_H
