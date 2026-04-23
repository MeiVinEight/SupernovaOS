#include <driver/xhci/xhci_port.h>
#include <core.h>
#include <timer/timer.h>
#include <console.h>
#include <stdio.h>

DWORD xhci_port_usb3(volatile PCI_EXPRESS_XHCI_CONTROLLER *device, DWORD portId)
{
	// Foreach xECP
	volatile QWORD xadr = device->address;
	volatile DWORD xecp = device->capability->XECP << 2;
	while (xecp)
	{
		xadr = xadr + xecp;
		volatile XHCI_EXTENDED_CAPABILITY *xcap = (XHCI_EXTENDED_CAPABILITY *) core_mapping(xadr);
		if (xcap->CAID == XHCI_XECP_SUPPORTED_PROTOCOL)
		{
			volatile XHCI_CAPABILITY_SUPPORTED_PROTOCOL *supp = (XHCI_CAPABILITY_SUPPORTED_PROTOCOL *) xcap;
			if ((supp->MAJV == 3) && ((supp->CPOF - 1) <= portId) && ((supp->CPOF - 1 + supp->CPCN) > portId))
				return 1;
		}
		xecp = xcap->NEXT << 2;
	}
	return 0;
}
DWORD xhci_port_reset(volatile PCI_EXPRESS_XHCI_CONTROLLER *device, DWORD portId)
{
	DWORD usb3 = xhci_port_usb3(device, portId);
	volatile XHCI_PORT_SPACE *port = device->operational->PORT + portId;
	device->status[portId] = 0;
	volatile XHCI_PORT_STATUS *status = (XHCI_PORT_STATUS *) device->status + portId;

	if (port->POEN)
	{
		status->RST = 0;
		return 0;
	}

	// Power on the port if necessary (spec requires PP=1 before any state change)
	if (!port->PPWR)
	{
		port->PTSC = (port->PTSC | XHCI_PORTSC_PPWR) & XHCI_PORTSC_RWX_MASK;
		delay(1000);
		if (!port->PPWR)
		{
			return 1;
		}
	}

	// Clear any lingering change bits before initiating the reset
	xhci_port_ack_port_changes(device, portId, XHCI_PORTSC_W1C_MASK);

	// Re-initiate the port reset
	if (usb3)
	{
		// Warm port reset for USB 3.x
		port->PTSC = (port->PTSC & XHCI_PORTSC_RWX_MASK) | XHCI_PORTSC_WRST;
	}
	else
	{
		// Standard port reset for USB 2.0
		port->PTSC = (port->PTSC & XHCI_PORTSC_RWX_MASK) | XHCI_PORTSC_PRST;
	}

	// Wait for reset completion (PRC for USB 2, WRC for USB 3)
	DWORD waitMs = 500;
	while (waitMs && !status->RST)
	{
		delay(1);
		waitMs--;
	}
	//if ((usb3 && !port->WRCH) || (!usb3 && !port->PRCH))
	if (!status->RST)
	{
		return 2;
	}
	status->RST = 0;

	delay(100); // Post-reset settling
	if (status->ERR)
	{
		return 3;
	}

	delay(100);
    // Verify the port is enabled after reset
	if (!port->POEN)
	{
		return 4;
	}
	return 0;
}
void xhci_port_ack_port_changes(volatile PCI_EXPRESS_XHCI_CONTROLLER *device, DWORD portId, DWORD changeMask)
{
	volatile XHCI_PORT_SPACE *port = device->operational->PORT + portId;
	DWORD portsc = port->PTSC;
	// preserve R/W bits, zero all RW1C
	portsc &= XHCI_PORTSC_RWX_MASK;
	// write-1-to-clear the targeted ones
	portsc |= changeMask & XHCI_PORTSC_W1C_MASK;
	port->PTSC = portsc;
	// flush posted writes
	(void) port->PTSC;
}