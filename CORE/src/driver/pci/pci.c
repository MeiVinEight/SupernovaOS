#include  <driver/pci/pci.h>
#include <console.h>
#include <core.h>

COREAPI char PCI_VENDOR_10DE[] = "NVIDIA Corporation";
COREAPI char PCI_VENDOR_1234[] = "QEMU Virtual Machine";
COREAPI char PCI_VENDOR_15AD[] = "VMware";
COREAPI char PCI_VENDOR_1969[] = "Qualcomm Atheros";
COREAPI char PCI_VENDOR_1987[] = "Phison Electronics Corporation";
COREAPI char PCI_VENDOR_1B21[] = "ASMedia Technology Inc.";
COREAPI char PCI_VENDOR_1B36[] = "Red Hat Inc";
COREAPI char PCI_VENDOR_1DEE[] = "Biwin Storage Technology Co., Ltd.";
COREAPI char PCI_VENDOR_8086[] = "Intel Corporation";
COREAPI char PCI_VENDOR_80EE[] = "InnoTek Systemberatung GmbH";

COREAPI char PCI_DEVICE_10DE10F7[] = "TU102 High Definition Audio Controller";
COREAPI char PCI_DEVICE_10DE1AD6[] = "TU102 USB 3.1 Host Controller";
COREAPI char PCI_DEVICE_10DE1AD7[] = "TU102 USB Type-C UCSI Controller";
COREAPI char PCI_DEVICE_10DE1E07[] = "TU102 [GeForce RTX 2080 Ti Rev. A]";

COREAPI char PCI_DEVICE_12341111[] = "VGA Controller";

COREAPI char PCI_DEVICE_15AD0405[] = "SVGA II Adapter";
COREAPI char PCI_DEVICE_15AD0740[] = "Virtual Machine Communication Interface";
COREAPI char PCI_DEVICE_15AD0770[] = "USB2 EHCI Controller";
COREAPI char PCI_DEVICE_15AD0774[] = "USB1.1 UHCI Controller";
COREAPI char PCI_DEVICE_15AD077A[] = "USB3 xHCI 2.0 Controller";
COREAPI char PCI_DEVICE_15AD0790[] = "PCI bridge";
COREAPI char PCI_DEVICE_15AD07A0[] = "PCI Express Root Port";
COREAPI char PCI_DEVICE_15AD07E0[] = "SATA AHCI controller";

COREAPI char PCI_DEVICE_1969E0B1[] = "Killer E2500 Gigabit Ethernet Controller";

COREAPI char PCI_DEVICE_19875013[] = "PS5013 E13 NVMe Controller";

COREAPI char PCI_DEVICE_1B212142[] = "ASM2142/ASM3142 USB 3.1 Host Controller";

COREAPI char PCI_DEVICE_1B36000D[] = "QEMU XHCI Host Controller";

COREAPI char PCI_DEVICE_1DEE2262[] = "HP EX950 NVMe SSD";

COREAPI char PCI_DEVICE_808606ED[] = "Comet Lake USB 3.1 xHCI Host Controller";
COREAPI char PCI_DEVICE_8086100E[] = "QEMU Virtual Machine Ethernet Controller";
COREAPI char PCI_DEVICE_808610D3[] = "QEMU Virtual Machine Ethernet Controller";
COREAPI char PCI_DEVICE_80861237[] = "440FX - 82441FX PMC [Natoma]";
COREAPI char PCI_DEVICE_80861901[] = "6th-9th Gen Core Processor PCIe Controller (x16)";
COREAPI char PCI_DEVICE_80861911[] = "Xeon E3-1200 v5/v6 / E3-1500 v5 / 6th/7th/8th Gen Core Processor Gaussian Mixture Model";
COREAPI char PCI_DEVICE_8086272B[] = "Wi-Fi 7(802.11be) AX1775*/AX1790*/BE20*/BE401/BE1750* 2x2";
COREAPI char PCI_DEVICE_808627B9[] = "VirtualBox ISA Bridge";
COREAPI char PCI_DEVICE_80862829[] = "82801HM/HEM (ICH8M/ICH8M-E) SATA Controller [AHCI mode]";
COREAPI char PCI_DEVICE_80862918[] = "QEMU Virtual Machine ISA Bridge";
COREAPI char PCI_DEVICE_80862922[] = "QEMU Virtual Machine SATA Controller [AHCI 1.0]";
COREAPI char PCI_DEVICE_80862930[] = "QEMU Virtual Machine SMBus Controller";
COREAPI char PCI_DEVICE_808629C0[] = "QEMU Virtual Machine Host Bridge";
COREAPI char PCI_DEVICE_80863EC2[] = "8th Gen Core Processor Host Bridge/DRAM Registers";
COREAPI char PCI_DEVICE_80867000[] = "82371SB PIIX3 ISA [Natoma/Triton II]";
COREAPI char PCI_DEVICE_80867010[] = "QEMU Virtual Machine IDE Controller";
COREAPI char PCI_DEVICE_80867110[] = "82371AB/EB/MB PIIX4 ISA";
COREAPI char PCI_DEVICE_80867111[] = "82371AB/EB/MB PIIX4 IDE";
COREAPI char PCI_DEVICE_80867113[] = "82371AB/EB/MB PIIX4 ACPI";
COREAPI char PCI_DEVICE_80867191[] = "440BX/ZX/DX - 82443BX/ZX/DX AGP bridge";
COREAPI char PCI_DEVICE_80867192[] = "440BX/ZX/DX - 82443BX/ZX/DX Host bridge (AGP disabled)";
COREAPI char PCI_DEVICE_8086A282[] = "200 Series PCH SATA controller [AHCI mode]";
COREAPI char PCI_DEVICE_8086A290[] = "200 Series PCH PCI Express Root Port #1";
COREAPI char PCI_DEVICE_8086A292[] = "200 Series PCH PCI Express Root Port #3";
COREAPI char PCI_DEVICE_8086A293[] = "200 Series PCH PCI Express Root Port #4";
COREAPI char PCI_DEVICE_8086A294[] = "200 Series PCH PCI Express Root Port #5";
COREAPI char PCI_DEVICE_8086A295[] = "200 Series PCH PCI Express Root Port #6";
COREAPI char PCI_DEVICE_8086A296[] = "200 Series PCH PCI Express Root Port #7";
COREAPI char PCI_DEVICE_8086A297[] = "200 Series PCH PCI Express Root Port #8";
COREAPI char PCI_DEVICE_8086A298[] = "200 Series PCH PCI Express Root Port #9";
COREAPI char PCI_DEVICE_8086A2A1[] = "200 Series/Z370 Chipset Family Power Management Controller";
COREAPI char PCI_DEVICE_8086A2A3[] = "200 Series/Z370 Chipset Family SMBus Controller";
COREAPI char PCI_DEVICE_8086A2AF[] = "200 Series/Z370 Chipset Family USB 3.0 xHCI Controller";
COREAPI char PCI_DEVICE_8086A2BA[] = "200 Series PCH CSME HECI #1";
COREAPI char PCI_DEVICE_8086A2E7[] = "200 Series PCH PCI Express Root Port #17";
COREAPI char PCI_DEVICE_8086A2E9[] = "200 Series PCH PCI Express Root Port #19";
COREAPI char PCI_DEVICE_8086A2EB[] = "200 Series PCH PCI Express Root Port #21";
COREAPI char PCI_DEVICE_8086A2C9[] = "Z370 Chipset LPC/eSPI Controller";
COREAPI char PCI_DEVICE_8086A2F0[] = "200 Series PCH HD Audio";

COREAPI char PCI_DEVICE_80EE4E56[] = "VirtualBox NVMe Controller";
COREAPI char PCI_DEVICE_80EEBEEF[] = "VirtualBox Graphics Adapter";
COREAPI char PCI_DEVICE_80EECAFE[] = "VirtualBox Guest Service";

const char *pci_vendor_name(DWORD vendorId)
{
	switch (vendorId)
	{
		case 0x10DE: return PCI_VENDOR_10DE;
		case 0x1234: return PCI_VENDOR_1234;
		case 0x15AD: return PCI_VENDOR_15AD;
		case 0x1969: return PCI_VENDOR_1969;
		case 0x1987: return PCI_VENDOR_1987;
		case 0x1B21: return PCI_VENDOR_1B21;
		case 0x1B36: return PCI_VENDOR_1B36;
		case 0x1DEE: return PCI_VENDOR_1DEE;
		case 0x8086: return PCI_VENDOR_8086;
		case 0x80EE: return PCI_VENDOR_80EE;
		default: return 0;
	}
}
const char *pci_device_name(PCI_DEVICE_VENDOR device)
{
	if (device.VENDOR == 0x10DE)
	{
		if (device.DEVICE == 0x10F7) return PCI_DEVICE_10DE10F7;
		if (device.DEVICE == 0x1AD6) return PCI_DEVICE_10DE1AD6;
		if (device.DEVICE == 0x1AD7) return PCI_DEVICE_10DE1AD7;
		if (device.DEVICE == 0x1E07) return PCI_DEVICE_10DE1E07;
	}
	if (device.VENDOR == 0x1234)
	{
		if (device.DEVICE == 0x1111) return PCI_DEVICE_12341111;
	}
	if (device.VENDOR == 0x1969)
	{
		if (device.DEVICE == 0xE0B1) return PCI_DEVICE_1969E0B1;
	}
	if (device.VENDOR == 0x1987)
	{
		if (device.DEVICE == 0x5013) return PCI_DEVICE_19875013;
	}
	if (device.VENDOR == 0x1B21)
	{
		if (device.DEVICE == 0x2142) return PCI_DEVICE_1B212142;
	}
	if (device.VENDOR == 0x1B36)
	{
		if (device.DEVICE == 0x000D) return PCI_DEVICE_1B36000D;
	}
	if (device.VENDOR == 0x1DEE)
	{
		if (device.DEVICE == 0x2262) return PCI_DEVICE_1DEE2262;
	}
	if (device.VENDOR == 0x15AD)
	{
		if (device.DEVICE == 0x0405) return PCI_DEVICE_15AD0405;
		if (device.DEVICE == 0x0740) return PCI_DEVICE_15AD0740;
		if (device.DEVICE == 0x0770) return PCI_DEVICE_15AD0770;
		if (device.DEVICE == 0x0774) return PCI_DEVICE_15AD0774;
		if (device.DEVICE == 0x077A) return PCI_DEVICE_15AD077A;
		if (device.DEVICE == 0x0790) return PCI_DEVICE_15AD0790;
		if (device.DEVICE == 0x07A0) return PCI_DEVICE_15AD07A0;
		if (device.DEVICE == 0x07E0) return PCI_DEVICE_15AD07E0;
	}
	if (device.VENDOR == 0x8086)
	{
		if (device.DEVICE == 0x06ED) return PCI_DEVICE_808606ED;
		if (device.DEVICE == 0x100E) return PCI_DEVICE_8086100E;
		if (device.DEVICE == 0x10D3) return PCI_DEVICE_808610D3;
		if (device.DEVICE == 0x1237) return PCI_DEVICE_80861237;
		if (device.DEVICE == 0x1901) return PCI_DEVICE_80861901;
		if (device.DEVICE == 0x1911) return PCI_DEVICE_80861911;
		if (device.DEVICE == 0x272B) return PCI_DEVICE_8086272B;
		if (device.DEVICE == 0x27B9) return PCI_DEVICE_808627B9;
		if (device.DEVICE == 0x2829) return PCI_DEVICE_80862829;
		if (device.DEVICE == 0x2918) return PCI_DEVICE_80862918;
		if (device.DEVICE == 0x2922) return PCI_DEVICE_80862922;
		if (device.DEVICE == 0x2930) return PCI_DEVICE_80862930;
		if (device.DEVICE == 0x29C0) return PCI_DEVICE_808629C0;
		if (device.DEVICE == 0x3EC2) return PCI_DEVICE_80863EC2;
		if (device.DEVICE == 0x7000) return PCI_DEVICE_80867000;
		if (device.DEVICE == 0x7010) return PCI_DEVICE_80867010;
		if (device.DEVICE == 0x7110) return PCI_DEVICE_80867110;
		if (device.DEVICE == 0x7111) return PCI_DEVICE_80867111;
		if (device.DEVICE == 0x7113) return PCI_DEVICE_80867113;
		if (device.DEVICE == 0x7191) return PCI_DEVICE_80867191;
		if (device.DEVICE == 0x7192) return PCI_DEVICE_80867192;
		if (device.DEVICE == 0xA282) return PCI_DEVICE_8086A282;
		if (device.DEVICE == 0xA290) return PCI_DEVICE_8086A290;
		if (device.DEVICE == 0xA292) return PCI_DEVICE_8086A292;
		if (device.DEVICE == 0xA293) return PCI_DEVICE_8086A293;
		if (device.DEVICE == 0xA294) return PCI_DEVICE_8086A294;
		if (device.DEVICE == 0xA295) return PCI_DEVICE_8086A295;
		if (device.DEVICE == 0xA296) return PCI_DEVICE_8086A296;
		if (device.DEVICE == 0xA297) return PCI_DEVICE_8086A297;
		if (device.DEVICE == 0xA298) return PCI_DEVICE_8086A298;
		if (device.DEVICE == 0xA2A1) return PCI_DEVICE_8086A2A1;
		if (device.DEVICE == 0xA2A3) return PCI_DEVICE_8086A2A3;
		if (device.DEVICE == 0xA2AF) return PCI_DEVICE_8086A2AF;
		if (device.DEVICE == 0xA2BA) return PCI_DEVICE_8086A2BA;
		if (device.DEVICE == 0xA2C9) return PCI_DEVICE_8086A2C9;
		if (device.DEVICE == 0xA2E7) return PCI_DEVICE_8086A2E7;
		if (device.DEVICE == 0xA2E9) return PCI_DEVICE_8086A2E9;
		if (device.DEVICE == 0xA2EB) return PCI_DEVICE_8086A2EB;
		if (device.DEVICE == 0xA2F0) return PCI_DEVICE_8086A2F0;
	}
	if (device.VENDOR == 0x80EE)
	{
		if (device.DEVICE == 0x4E56) return PCI_DEVICE_80EE4E56;
		if (device.DEVICE == 0xBEEF) return PCI_DEVICE_80EEBEEF;
		if (device.DEVICE == 0xCAFE) return PCI_DEVICE_80EECAFE;
	}
	return 0;
}
DWORD pci_read_config_word(DWORD deviceId, DWORD offset)
{
	if (deviceId & 0xFF)
		return -1;
	if (offset > 0xFF)
		return -1;
	if (offset & 1)
		return -1;

	// Read aligned dword
	DWORD address = deviceId | (offset & 0xFC);
	__outdword(PCI_CONFIG_ADDRESS, address);
	DWORD value = __indword(PCI_CONFIG_DATA);

	// Extract the correct word from the dword
	BYTE shift = (BYTE) ((offset & 2) << 3);
	return (value >> shift) & 0xFFFF;
}
DWORD pci_read_config_dword(DWORD deviceId, DWORD offset)
{
	if (deviceId & 0xFF)
		return -1;
	if (offset > 0xFF)
		return -1;
	if (offset & 3)
		return -1;
	DWORD address = deviceId | (offset & 0xFC);
	__outdword(PCI_CONFIG_ADDRESS, address);
	return __indword(PCI_CONFIG_DATA);
}
void pci_write_config_word(DWORD deviceId, DWORD offset, DWORD value)
{
	if (deviceId & 0xFF)
		return;
	if (offset > 0xFF)
		return;
	if (offset & 1)
		return;

	value &= 0xFFFF;
	DWORD address = deviceId | (offset & 0xFC);
	__outdword(PCI_CONFIG_ADDRESS, address);
	DWORD oldVal = __indword(PCI_CONFIG_DATA);
	BYTE shift = ((offset & 2) << 3);
	DWORD mask = 0xFFFF << shift;
	DWORD newVal = (oldVal & ~mask) | (value << shift);

	__outdword(PCI_CONFIG_ADDRESS, newVal);
	__outdword(PCI_CONFIG_DATA, newVal);
}
void pci_write_config_dword(DWORD deviceId, DWORD offset, DWORD value)
{
	if (offset & 3)
		return;
	if (offset > 0xFF)
		return;
	if (deviceId & 0xFF)
		return;

	DWORD address = deviceId | offset;
	__outdword(PCI_CONFIG_ADDRESS, address);
	__outdword(PCI_CONFIG_DATA, value);
}
DWORD pci_cfg_get_command(DWORD deviceId)
{
	return pci_read_config_word(deviceId, PCI_OFFSET_COMMAND);
}
void pci_cfg_set_command(DWORD deviceId, DWORD value)
{
	pci_write_config_word(deviceId, PCI_OFFSET_COMMAND, value);
}
PCI_DEVICE_VENDOR pci_cfg_get_vendor(PCI_DEVICE_ADDRESS address)
{
	PCI_DEVICE_VENDOR deviceId;
	deviceId.ID = pci_read_config_dword(address.address, PCI_OFFSET_VENDOR);
	return deviceId;
}
DWORD pci_cfg_get_class(PCI_DEVICE_ADDRESS address)
{
	return pci_read_config_dword(address.address, PCI_OFFSET_REVISION) >> 8;
}
DWORD pci_cfg_get_header_type(PCI_DEVICE_ADDRESS address)
{
	return pci_read_config_word(address.address, PCI_OFFSET_HEADER_TYPE) & 0xFF;
}
PCI_DEVICE_SUBSYSTEM pci_cfg_get_subsystem(PCI_DEVICE_ADDRESS address)
{
	PCI_DEVICE_SUBSYSTEM subsystem;
	subsystem.value = pci_read_config_dword(address.address, PCI_OFFSET_SUBSYSTEM);
	return subsystem;
}
void pci_enable(PCI_DEVICE_ADDRESS deviceId)
{
	DWORD command = pci_cfg_get_command(deviceId.address);
	if (command == -1)
		return;

	command |= (PCI_COMMAND_IO_SPACE | PCI_COMMAND_MEMORY_SPACE);
	pci_cfg_set_command(deviceId.address, command);
}
void setup_pci()
{
	PCI_DEVICE_ADDRESS addr;
	addr.address = 0;
	addr.ENABLE = 1;
	for (; !addr.RSV; addr.address += 0x100)
	{
		PCI_DEVICE_VENDOR vendor = pci_cfg_get_vendor(addr);
		if ((!vendor.VENDOR) || (vendor.VENDOR == 0xFFFF))
			continue;
		DWORD classCode = pci_cfg_get_class(addr);
		PCI_DEVICE_SUBSYSTEM subsystem = pci_cfg_get_subsystem(addr);
		simple_output("PCI @ ");
		simple_output_address(addr.address, 8);
		outchar('-');
		simple_output_address(subsystem.value, 8);
		simple_output(": ");
		simple_output_address(classCode, 6);
		simple_output(" - ");
		const char *vendorName = pci_vendor_name(vendor.VENDOR);
		const char *deviceName = pci_device_name(vendor);
		if (vendorName && deviceName)
		{
			simple_output(vendorName);
			outchar(' ');
			simple_output(deviceName);
		}
		else
			simple_output_address(vendor.ID, 8);
		outchar('\n');
	}
}