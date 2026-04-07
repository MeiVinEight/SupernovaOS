//
// Created by MeiVi on 2026/04/07.
//

#pragma once

#include <driver/usb/usb_desc.h>
#include <driver/xhci/xhci_device.h>

typedef struct _HID_STANDARD_KEYEVENT
{
	BYTE MOD;
	BYTE RSV;
	BYTE KEY[6];
} HID_STANDARD_KEYEVENT;

DWORD xhci_usb_keyboard_setup(XHCI_USB_DEVICE *device, STANDARD_USB_INTERFACE *iface);
void xhci_keyboard_process();