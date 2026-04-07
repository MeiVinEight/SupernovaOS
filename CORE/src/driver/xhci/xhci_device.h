//
// Created by MeiVi on 2026/04/01.
//

#ifndef SUPERNOVAOS_XHCI_DEVICE_H
#define SUPERNOVAOS_XHCI_DEVICE_H

#include <types.h>
#include <driver/xhci/xhc_ring.h>
#include <driver/usb/usb_desc.h>

/*
// xHci Spec Section 7.2.1 Protocol Speed ID (PSI) (page 524)

Protocol Speed ID (PSI) Dwords immediately follow the Dword at offset 10h in
an xHCI Supported Protocol Capability data structure. Table 7-10 defines the
fields of a PSI Dword.
*/
#define XHCI_USB_SPEED_UNDEFINED            0
#define XHCI_USB_SPEED_FULL_SPEED           1 // 12 MB/s USB 2.0
#define XHCI_USB_SPEED_LOW_SPEED            2 // 1.5 Mb/s USB 2.0
#define XHCI_USB_SPEED_HIGH_SPEED           3 // 480 Mb/s USB 2.0
#define XHCI_USB_SPEED_SUPER_SPEED          4 // 5 Gb/s (Gen1 x1) USB 3.0
#define XHCI_USB_SPEED_SUPER_SPEED_PLUS     5 // 10 Gb/s (Gen2 x1) USB 3.1

typedef struct _XHCI_USB_DEVICE
{
	// XHCI Controller
	void *controller;
	struct _XHCI_USB_DEVICE *parent;
	// Input Context
	void *input;
	STANDARD_USB_CONFIGURATION *configuration;
	QWORD persistent;
	// Transfer Ring
	XHCI_TRANSFER_RING transfer;
	// Route String
	DWORD route;
	// Device Port
	BYTE port;
	// Device Root Port Id
	BYTE root;
	// Device Slot
	BYTE slot;
	// Port Speed ID
	BYTE speed;
} XHCI_USB_DEVICE;

DWORD xhci_setup_usb_device(XHCI_USB_DEVICE *device, DWORD portId, DWORD slotId);
void xhci_usb_enumerate_device(XHCI_USB_DEVICE *device);
DWORD xhci_usb_hid_setup(XHCI_USB_DEVICE *device, STANDARD_USB_INTERFACE *iface);
DWORD xhci_send_control_transfer(volatile XHCI_USB_DEVICE *device, USB_DEVICE_SETUP_DATA *requ, void *buf, QWORD len);

#endif //SUPERNOVAOS_XHCI_DEVICE_H
