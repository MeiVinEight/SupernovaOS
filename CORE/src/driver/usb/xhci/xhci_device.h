//
// Created by MeiVi on 2026/04/01.
//

#ifndef SUPERNOVAOS_XHCI_DEVICE_H
#define SUPERNOVAOS_XHCI_DEVICE_H

#include <types.h>

typedef struct _XHCI_USB_DEVICE
{
	void *controller;
} XHCI_USB_DEVICE;

DWORD setup_usb_device(XHCI_USB_DEVICE *device);

#endif //SUPERNOVAOS_XHCI_DEVICE_H
