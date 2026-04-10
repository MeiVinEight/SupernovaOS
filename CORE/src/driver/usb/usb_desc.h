//
// Created by MeiVi on 2026/04/02.
//

#ifndef SUPERNOVAOS_USB_DESC_H
#define SUPERNOVAOS_USB_DESC_H

#include <types.h>

#define USB_DESC_DEVICE                    0x01
#define USB_DESC_CONFIGURATION             0x02
#define USB_DESC_STRING                    0x03
#define USB_DESC_INTERFACE                 0x04
#define USB_DESC_ENDPOINT                  0x05
#define USB_DESC_DEVICE_QUALIFIER          0x06
#define USB_DESC_OTHER_SPEED_CONFIGURATION 0x07
#define USB_DESC_INTERFACE_POWER           0x08

#define USB_XFER_TYPE_CTRL  0
#define USB_XFER_TYPE_ISOCH 1
#define USB_XFER_TYPE_BULK  2
#define USB_XFER_TYPE_INT   3

#define USB_DIR_OUT   0     /* to device */
#define USB_DIR_IN    0x80  /* to host */

#define USB_ENDPOINT_MASK_NUM  0x0f    /* in bEndpointAddress */
#define USB_ENDPOINT_MASK_DIR  0x80

#define USB_ENDPOINT_XFER_TYPE 0x03    /* in bmAttributes */

#define USB_CLASS_HID           0x03
#define USB_CLASS_MASS_STORAGE  0x08

#define USB_HID_SUBCLASS_BOOT     1
#define USB_HID_PROTOCOL_KEYBOARD 1
#define USB_HID_PROTOCOL_MOUSE    2

#define USB_MSC_SUBCLASS_SCSI  0x06
#define USB_MSC_PROTOCOL_BOT   0x50
#define USB_MSC_PROTOCOL_UAS   0x62

typedef struct _STANDARD_USB_DEVICE
{
	// Size of this descriptor in bytes
	BYTE  LENG;
	// DEVICE Descriptor Type
	BYTE  TYPE;
	/**
	 * USB Specification Release Number in
	 * Binary-Coded Decimal (i.e., 2.10 is 210H).
	 * This field identifies the release of the USB
	 * Specification with which the device and its
	 * descriptors are compliant.
	 */
	WORD  UREL;
	/**
	 * Class code (assigned by the USB-IF).
	 *
	 * If this field is reset to zero, each interface
	 * within a configuration specifies its own
	 * class information and the various
	 * interfaces operate independently.
	 *
	 * If this field is set to a value between 1 and
	 * FEH, the device supports different class
	 * specifications on different interfaces and
	 * the interfaces may not operate
	 * independently. This value identifies the
	 * class definition used for the aggregate
	 * interfaces.
	 *
	 * If this field is set to FFH, the device class
	 * is vendor-specific.
	 */
	BYTE  CCOD;
	/**
	 * Subclass code (assigned by the USB-IF).
	 *
	 * These codes are qualified by the value of
	 * the bDeviceClass field.
	 *
	 * If the bDeviceClass field is reset to zero,
	 * this field must also be reset to zero.
	 *
	 * If the bDeviceClass field is not set to FFH,
	 * all values are reserved for assignment by
	 * the USB-IF.
	 */
	BYTE  SCOD;
	/**
	 * Protocol code (assigned by the USB-IF).
	 *
	 * These codes are qualified by the value of
	 * the bDeviceClass and the
	 * bDeviceSubClass fields. If a device
	 * supports class-specific protocols on a
	 * device basis as opposed to an interface
	 * basis, this code identifies the protocols
	 * that the device uses as defined by the
	 * specification of the device class.
	 *
	 * If this field is reset to zero, the device
	 * does not use class-specific protocols on a
	 * device basis. However, it may use class specific protocols on an interface basis.
	 *
	 * If this field is set to FFH, the device uses a
	 * vendor-specific protocol on a device basis.
	 */
	BYTE  POTO;
	// Maximum packet size for endpoint zero (only 8, 16, 32, or 64 are valid)
	BYTE  MPS0;
	// Vendor ID (assigned by the USB-IF)
	WORD  VNID;
	// Product ID (assigned by the manufacturer)
	WORD  PRID;
	// Device release number in binary-coded decimal
	WORD  DREL;
	// Index of string descriptor describing manufacturer
	BYTE  MANI;
	// Index of string descriptor describing product
	BYTE  PRDI;
	// Index of string descriptor describing the device's serial number
	BYTE  SNUM;
	// Number of possible configurations
	BYTE  CNFN;
} STANDARD_USB_DEVICE;
typedef struct _STANDARD_USB_CONFIGURATION
{
	// Size of this descriptor in bytes
	BYTE LENG;
	// CONFIGURATION Descriptor Type
	BYTE TYPE;
	/**
	 * Total length of data returned for this
	 * configuration. Includes the combined length
	 * of all descriptors (configuration, interface,
	 * endpoint, and class- or vendor-specific)
	 * returned for this configuration.
	 */
	WORD TLEN;
	// Number of interfaces supported by this configuration
	BYTE IFAC;
	/**
	 * Value to use as an argument to the
	 * SetConfiguration() request to select this
	 * configuration
	 */
	BYTE CNFV;
	// Index of string descriptor describing this configuration
	BYTE CFGI;
	/**
	 * Configuration characteristics
	 * D7: Reserved (set to one)
	 * D6: Self-powered
	 * D5: Remote Wakeup
	 * D4...0: Reserved (reset to zero)
	 * D7 is reserved and must be set to one for
	 * historical reasons.
	 *
	 * A device configuration that uses power from
	 * the bus and a local source reports a non-zero
	 * value in bMaxPower to indicate the amount of
	 * bus power required and sets D6. The actual
	 * power source at runtime may be determined
	 * using the GetStatus(DEVICE) request (see
	 * Section 9.4.5).
	 *
	 * If a device configuration supports remote
	 * wakeup, D5 is set to one.
	 */
	BYTE ATTR;
	/**
	 * Maximum power consumption of the USB
	 * device from the bus in this specific
	 * configuration when the device is fully
	 * operational. Expressed in 2 mA units
	 * (i.e., 50 = 100 mA).
	 *
	 * Note: A device configuration reports whether
	 * the configuration is bus-powered or self powered.
	 *
	 * Device status reports whether the
	 * device is currently self-powered. If a device is
	 * disconnected from its external power source, it
	 * updates device status to indicate that it is no
	 * longer self-powered.
	 *
	 * A device may not increase its power draw
	 * from the bus, when it loses its external power
	 * source, beyond the amount reported by its
	 * configuration.
	 *
	 * If a device can continue to operate when
	 * disconnected from its external power source, it
	 * continues to do so. If the device cannot
	 * continue to operate, it fails operations it can
	 * no longer support. The USB System Software
	 * may determine the cause of the failure by
	 * checking the status and noting the loss of the
	 * device’s power source.
	 */
	BYTE MPWR;
	BYTE DATA[];
} STANDARD_USB_CONFIGURATION;
typedef struct _STANDARD_USB_STRING
{
	// Size of this descriptor in bytes
	BYTE LENG;
	// STRING Descriptor Type
	BYTE TYPE;
	// LANGID code
	WORD DATA[];
} STANDARD_USB_STRING;
typedef struct _STANDARD_USB_INTERFACE
{
	// Size of this descriptor in bytes
	BYTE LENG;
	// INTERFACE Descriptor Type
	BYTE TYPE;
	/**
	 * Number of this interface. Zero-based
	 * value identifying the index in the array of
	 * concurrent interfaces supported by this
	 * configuration.
	 */
	BYTE IFCN;
	// Value used to select this alternate setting for the interface identified in the prior field
	BYTE SETT;
	/**
	 * Number of endpoints used by this
	 * interface (excluding endpoint zero). If this
	 * value is zero, this interface only uses the
	 * Default Control Pipe.
	 */
	BYTE ENDP;
	/**
	 * Class code (assigned by the USB-IF).
	 * A value of zero is reserved for future
	 * standardization.
	 *
	 * If this field is set to FFH, the interface
	 * class is vendor-specific.
	 *
	 * All other values are reserved for
	 * assignment by the USB-IF.
	 */
	BYTE CCOD;
	/**
	 * Subclass code (assigned by the USB-IF).
	 * These codes are qualified by the value of
	 * the bInterfaceClass field.
	 *
	 * If the bInterfaceClass field is reset to zero,
	 * this field must also be reset to zero.
	 *
	 * If the bInterfaceClass field is not set to
	 * FFH, all values are reserved for
	 * assignment by the USB-IF.
	 */
	BYTE SCOD;
	/**
	 * rotocol code (assigned by the USB).
	 * These codes are qualified by the value of
	 * the bInterfaceClass and the
	 * bInterfaceSubClass fields. If an interface
	 * supports class-specific requests, this code
	 * identifies the protocols that the device
	 * uses as defined by the specification of the
	 * device class.
	 *
	 * If this field is reset to zero, the device
	 * does not use a class-specific protocol on
	 * this interface.
	 *
	 * If this field is set to FFH, the device uses
	 * a vendor-specific protocol for this
	 * interface.
	 */
	BYTE POTO;
	// Index of string descriptor describing this interface
	BYTE IFAC;
} STANDARD_USB_INTERFACE;
typedef struct _STANDARD_USB_ENDPOINT
{
	// Size of this descriptor in bytes
	BYTE LENG;
	// ENDPOINT Descriptor Type
	BYTE TYPE;
	/**
	 * The address of the endpoint on the USB device
	 * described by this descriptor. The address is
	 * encoded as follows:
	 * - Bit 3...0: The endpoint number
	 * - Bit 6...4: Reserved, reset to zero
	 * - Bit 7: Direction, ignored for
	 *
	 *  control endpoints
	 *  - 0 = OUT endpoint
	 *  - 1 = IN endpoint
	 */
	BYTE ADDR;
	/**
	 * This field describes the endpoint’s attributes when it is
	 * configured using the bConfigurationValue.
	 *
	 * Bits 1..0: Transfer Type
	 * - 00 = Control
	 * - 01 = Isochronous
	 * - 10 = Bulk
	 * - 11 = Interrupt
	 *
	 * If not an isochronous endpoint, bits 5..2 are reserved
	 * and must be set to zero. If isochronous, they are
	 * defined as follows:
	 *
	 * Bits 3..2: Synchronization Type
	 * - 00 = No Synchronization
	 * - 01 = Asynchronous
	 * - 10 = Adaptive
	 * - 11 = Synchronous
	 *
	 * Bits 5..4: Usage Type
	 * - 00 = Data endpoint
	 * - 01 = Feedback endpoint
	 * - 10 = Implicit feedback Data endpoint
	 * - 11 = Reserved
	 *
	 * Refer to Chapter 5 for more information.
	 *
	 * All other bits are reserved and must be reset to zero.
	 *
	 * Reserved bits must be ignored by the host.
	 */
	BYTE ATTR;
	/**
	 * Maximum packet size this endpoint is capable of
	 * sending or receiving when this configuration is
	 * selected.
	 *
	 * For isochronous endpoints, this value is used to
	 * reserve the bus time in the schedule, required for the
	 * per-(micro)frame data payloads. The pipe may, on an
	 * ongoing basis, actually use less bandwidth than that
	 * reserved. The device reports, if necessary, the actual
	 * bandwidth used via its normal, non-USB defined
	 * mechanisms.
	 *
	 * For all endpoints, bits 10..0 specify the maximum
	 * packet size (in bytes).
	 *
	 * For high-speed isochronous and interrupt endpoints:
	 * Bits 12..11 specify the number of additional transaction
	 * opportunities per microframe:
	 * - 00 = None (1 transaction per microframe)
	 * - 01 = 1 additional (2 per microframe)
	 * - 10 = 2 additional (3 per microframe)
	 * - 11 = Reserved
	 *
	 * Bits 15..13 are reserved and must be set to zero.
	 * Refer to Chapter 5 for more information.
	 */
	WORD MPSZ;
	/**
	 * Interval for polling endpoint for data transfers.
	 * Expressed in frames or microframes depending on the
	 * device operating speed (i.e., either 1 millisecond or
	 * 125 µs units).
	 *
	 * For full-/high-speed isochronous endpoints, this value
	 * must be in the range from 1 to 16. The bInterval value
	 * is used as the exponent for a 2^{bInterval-1} value; e.g., a
	 * bInterval of 4 means a period of 8 (2^{4-1}).
	 *
	 * For full-/low-speed interrupt endpoints, the value of
	 * this field may be from 1 to 255.
	 *
	 * For high-speed interrupt endpoints, the bInterval value
	 * is used as the exponent for a 2^{bInterval-1} value; e.g., a
	 * bInterval of 4 means a period of 8 (2^{4-1}). This value
	 * must be from 1 to 16.
	 *
	 * For high-speed bulk/control OUT endpoints, the
	 * bInterval must specify the maximum NAK rate of the
	 * endpoint. A value of 0 indicates the endpoint never
	 * NAKs. Other values indicate at most 1 NAK each
	 * bInterval number of microframes. This value must be
	 * in the range from 0 to 255.
	 *
	 * See Chapter 5 description of periods for more detail.
	 */
	BYTE ITVL;
} STANDARD_USB_ENDPOINT;

STANDARD_USB_ENDPOINT *usb_search_endpoint(STANDARD_USB_CONFIGURATION *conf, int type, int dir);

#endif //SUPERNOVAOS_USB_DESC_H
