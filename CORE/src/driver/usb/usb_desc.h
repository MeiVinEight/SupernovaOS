//
// Created by MeiVi on 2026/04/02.
//

#ifndef SUPERNOVAOS_USB_DESC_H
#define SUPERNOVAOS_USB_DESC_H

#include <types.h>

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

#endif //SUPERNOVAOS_USB_DESC_H
