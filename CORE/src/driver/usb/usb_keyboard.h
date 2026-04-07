//
// Created by MeiVi on 2026/04/07.
//

#pragma once

#include <driver/usb/usb_desc.h>
#include <driver/xhci/xhci_device.h>

DWORD xhci_usb_keyboard_setup(XHCI_USB_DEVICE *device, STANDARD_USB_INTERFACE *iface);
void xhci_keyboard_process();