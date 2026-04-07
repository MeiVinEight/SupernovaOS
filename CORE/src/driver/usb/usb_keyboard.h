//
// Created by MeiVi on 2026/04/07.
//

#pragma once

#include <driver/usb/usb_desc.h>
#include <driver/xhci/xhci_device.h>

#define KEYEVENT_CTRL_LCTRL  0x01
#define KEYEVENT_CTRL_LSHIFT 0x02
#define KEYEVENT_CTRL_LALT   0x04
#define KEYEVENT_CTRL_WIN    0x08
#define KEYEVENT_CTRL_RCTRL  0x10
#define KEYEVENT_CTRL_RSHIFT 0x20
#define KEYEVENT_CTRL_RALT   0x40

#define KEYEVENT_KEY_ESC     0x29

typedef struct _HID_STANDARD_KEYEVENT
{
	BYTE MOD;
	BYTE RSV;
	BYTE KEY[6];
} HID_STANDARD_KEYEVENT;

DWORD xhci_usb_keyboard_setup(XHCI_USB_DEVICE *device, STANDARD_USB_INTERFACE *iface);
void xhci_keyboard_process();