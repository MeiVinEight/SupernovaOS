//
// Created by MeiVi on 2026/03/18.
//

#ifndef SUPERNOVA_XHCI_H
#define SUPERNOVA_XHCI_H

#include <driver/pci/pcie.h>
#include <driver/usb/xhci/xhc_ring.h>

typedef struct _XHCI_CAPABILITY_SPACE
{
	BYTE SIZE;
	BYTE RSV0;
	WORD VRSN;
	//DWORD STR1;
	DWORD SLOT:8; // Max Number of Device Slots
	DWORD INTE:11;
	DWORD RSV1:5;
	DWORD PORT:8;
	//DWORD STR2;
	DWORD ISTH:4;
	DWORD ERST:4;
	DWORD RSV2:13;
	DWORD MSBH:5;
	DWORD SPDR:1;
	DWORD MSBL:5;
	DWORD STR3;
	//DWORD CAP1;
	DWORD AC64:1;
	DWORD BWNC:1;
	DWORD CX64:1;
	DWORD PPWC:1;
	DWORD PIND:1;
	DWORD LHCR:1;  // Light Host Controller Reset
	DWORD LTMC:1;  // Latency Tolerance Messaging Capability
	DWORD NSSS:1;  // No Secondary SID Support
	DWORD PAED:1;  // Parse All Event Data
	DWORD SSPC:1;  // Stopped - Short Packet Capability
	DWORD SEDC:1;  // Stopped EDTLA Capability
	DWORD CFIC:1;  // Contiguous Frame ID Capability
	DWORD PSAS:4;  // Maximum Primary Stream Array Size
	DWORD XECP:16; // xHCI Extended Capabilities Pointer
	DWORD BELL;    // Doorbell Offset
	DWORD RTME;    // Runtime Offset
	DWORD CAP2;
} XHCI_CAPABILITY_SPACE;
typedef struct _XHCI_OPERATIONAL_SPACE
{
	//DWORD command;
	DWORD RNST:1; // Run/Stop
	DWORD HRST:1; // Host Controller Reset
	DWORD INTE:1; // Interrupter Enable
	DWORD HSSE:1; // Host System Error Enable
	DWORD RSV0:3;
	DWORD LHCR:1; // Light Host Controller Reset
	DWORD COSS:1; // Controller Save Status
	DWORD CORS:1; // Controller Resotre Status
	DWORD ENWE:1; // Enable Wrap Event
	DWORD EU3S:1; // Enable U3 MFINDEX Stop
	DWORD RSV1:1;
	DWORD ECEM:1; // Enable CEM
	DWORD ETBC:1; // Enable Extended TBC
	DWORD ETTS:1; // Enable Extended TBC TRB Status
	DWORD VTIO:1; // Enable VTIO
	DWORD RSV2:15;
	//DWORD status;
	DWORD HALT:1; // Halted
	DWORD RSV3:1;
	DWORD SYSE:1; // Host System Error
	DWORD EINT:1; // Event Interrupter
	DWORD PCDT:1; // Port Change Detect
	DWORD RSV4:3;
	DWORD SSST:1; // Save State Status
	DWORD RSST:1; // Restore State Status
	DWORD SRER:1; // Save/Restore Error
	DWORD HCNR:1; // Controller Not Ready
	DWORD CTRE:1; // Host Controller Error
	DWORD RSV5:19;
	DWORD PAGE;
	DWORD RSV6[2];
	DWORD DNCR; // Device Notification Control Register
	QWORD CRCR;
	DWORD RSV7[4];
	QWORD CBAA;
	//DWORD configure;
	DWORD MDSE:8; // Max Device Slots Enabled
	DWORD ENU3:1; // Enable U3 Entry
	DWORD ENCI:1; // Enable Configuration Information
	DWORD RSV8:22;
	DWORD RSV9[0xF1];
} XHCI_OPERATIONAL_SPACE;
typedef struct _PCIE_XHCI_DEVICE
{
	volatile PCIE_DEVICE *pcie;
	volatile XHCI_CAPABILITY_SPACE *capability;
	volatile XHCI_OPERATIONAL_SPACE *operational;
	volatile QWORD *context;
	XHCI_TRANSFER_RING CMMD;
} PCIE_XHCI_DEVICE;

void setup_usb_xhci_pcie(volatile PCIE_DEVICE *device);
QWORD xhci_get_scratchpad_buffer(volatile PCIE_XHCI_DEVICE *device);
DWORD xhci_operational_command(volatile PCIE_XHCI_DEVICE *device);
DWORD xhci_operational_status(volatile PCIE_XHCI_DEVICE *device);
DWORD xhci_operational_config(volatile PCIE_XHCI_DEVICE *device);
DWORD xhci_reset_controller(volatile PCIE_XHCI_DEVICE *device);
void xhci_configure_operational(volatile PCIE_XHCI_DEVICE *device);

#endif //SUPERNOVA_XHCI_H
