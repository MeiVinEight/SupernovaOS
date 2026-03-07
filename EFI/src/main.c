
#include "uefi.h"
#include "intrinsic.h"
#include "pci.h"

#define PCI_COMMAND         0x04
#define PCI_BASE_ADDRESS_5	0x24

#define PCI_COMMAND_MEMORY  0x2
#define PCI_COMMAND_MASTER  0x4

#define HOST_CTL_AHCI_EN    (1 << 31)

#define HBA_PORT_IPM_ACTIVE  1
#define HBA_PORT_DET_PRESENT 3

#define HBA_PORT_CMD_ST      0x0001
#define HBA_PORT_CMD_SPIN_UP 0x0002
#define HBA_PORT_CMD_FRE     0x0010
#define HBA_PORT_CMD_FR      0x4000
#define HBA_PORT_CMD_CR      0x8000

#define	SATA_SIG_ATA    0x00000101  // SATA drive
#define	SATA_SIG_ATAPI  0xEB140101  // SATAPI drive
#define	SATA_SIG_SEMB   0xC33C0101  // Enclosure management bridge
#define	SATA_SIG_PM     0x96690101  // Port multiplier 

#define AHCI_DEVICE_NULL   0
#define AHCI_DEVICE_SATA   1
#define AHCI_DEVICE_SEMB   2
#define AHCI_DEVICE_PM     3
#define AHCI_DEVICE_SATAPI 4

#define ATA_CMD_READ_DMA_EX                 0x25

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ  0x08

#define HBA_PxIS_TFES   (1 << 30)       // TFES - Task File Error Status

#define CRT_ADDR_REG 0x3D4
#define CRT_DATA_REG 0x3D5

#define CRT_START_ADDR_H 0xC
#define CRT_START_ADDR_L 0xD
#define CRT_CURSOR_H 0xE
#define CRT_CURSOR_L 0xF

typedef struct _MEMORY_REGION
{
	QWORD A;
	QWORD L;
	DWORD F;
	DWORD X;
} MEMORY_REGION;

UEFIAPI QWORD EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID[2] = { 0x4A3823DC9042A9DE, 0x6A5180D0DE7AFB96 };
UEFIAPI QWORD EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID[2] = { 0x11d26459964e5b22, 0x3b7269c9a000398e };
UEFIAPI QWORD EFI_FILE_INFO_GUID[2] = { 0x11d26d3f09576e92, 0x3b7269c9a000398E };
UEFIAPI QWORD EFI_ACPI_20_TABLE_GUID[2] = { 0x11D3E4F18868E871, 0x81883CC7800022BC };
//UEFIAPI __declspec(align(4096)) BYTE paging[4096];

extern char __ImageBase;

void OutputText(const char *s)
{
	WORD buf[2] = { 0, 0 };
	while (*s)
	{
		buf[0] = (unsigned char) *s++;
		OutputString(buf);
	}
}
void OutputNumber(QWORD x)
{
	char buf[20] = { '0', 0 };
	char *num = buf;
	if (x)
	{
		buf[19] = 0;
		int idx = 19;
		while (idx && x)
		{
			buf[--idx] = '0' + (x % 10);
			x /= 10;
		}
		num += idx;
	}
	OutputText(num);
}
void OutputAddressX(QWORD val, BYTE cnt)
{
	char hex[] = "0123456789ABCDEF";
	char buf[17];
	buf[cnt] = 0;
	for (int i = cnt; i--;)
	{
		buf[i] = hex[val & 0xF];
		val >>= 4;
	}
	OutputText(buf);
}
void OutputAddress(QWORD addr)
{
	/*
	char hex[] = "0123456789ABCDEF";
	char buf[17];
	buf[16] = 0;
	for (int i = 16; i--;)
	{
		buf[i] = hex[addr & 0xF];
		addr >>= 4;
	}
	OutputText(buf);
	*/
	OutputAddressX(addr, 16);
}
QWORD jmp(QWORD(*entry)(), QWORD stackAddr)
{
	__cli();
	char idtr[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	__lidt(idtr);
	if(!entry)
		while (1) __halt();

	__setrsp(stackAddr - 0x28);
	return entry();
}

EFI_GRAPHICS_OUTPUT_MODE_INFORMATION DetectingMode(EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics)
{
	int defaultMode = graphics->Mode->Mode;
	QWORD sqr = graphics->Mode->Info->HorizontalResolution * graphics->Mode->Info->VerticalResolution;
	int usedMode = defaultMode;
	char sep[2] = { 0, 0 };
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION ret = {0, 0, 0, 0, 0};
	for (int i = 0; i < graphics->Mode->MaxMode; i++)
	{
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
		QWORD sizeOfInfo;
		graphics->QueryMode(graphics, i, &sizeOfInfo, &info);
		//graphics->SetMode(graphics, i);
		QWORD x = info->HorizontalResolution * info->VerticalResolution;
		if (x > sqr)
		{
			sqr = x;
			usedMode = i;
			ret = *info;
		}
	}
	graphics->SetMode(graphics, usedMode);
	return ret;
}
void SortMemoryMap(QWORD map, DWORD count, QWORD descSize)
{
	void *desc = 0;
	SYSTEM_TABLE->BootServices->AllocatePool(EfiBootServicesData, descSize, &desc);
	for (DWORD i = 0; i < count; i++)
	{
		DWORD min = i;
		EFI_MEMORY_DESCRIPTOR *minEntry = (EFI_MEMORY_DESCRIPTOR *) (map + min * descSize);
		for (DWORD j = i + 1; j < count; j++)
		{
			EFI_MEMORY_DESCRIPTOR *entry = (EFI_MEMORY_DESCRIPTOR *) (map + j * descSize);
			if (entry->PhysicalStart < minEntry->PhysicalStart)
			{
				min = j;
				minEntry = entry;
			}
		}
		if (min != i)
		{
			memcpy(desc, (void *) (map + i * descSize), descSize);
			memcpy((void *) (map + i * descSize), minEntry, descSize);
			memcpy(minEntry, desc, descSize);
		}
	}
	SYSTEM_TABLE->BootServices->FreePool(desc);
}
DWORD CompactMemoryMap(QWORD map, DWORD count, QWORD descSize)
{
	DWORD i = 0;
	EFI_MEMORY_DESCRIPTOR *ei = (EFI_MEMORY_DESCRIPTOR *) (map + i * descSize);
	for (DWORD j = 1; j < count; j++)
	{
		EFI_MEMORY_DESCRIPTOR *ej = (EFI_MEMORY_DESCRIPTOR *) (map + j * descSize);
		if (ej->Type == ei->Type && (ei->PhysicalStart + ei->NumberOfPages * 4096 == ej->PhysicalStart))
		{
			ei->NumberOfPages += ej->NumberOfPages;
		}
		else
		{
			i++;
			ei = (EFI_MEMORY_DESCRIPTOR *) (map + i * descSize);
			if (i != j)
			{
				memcpy(ei, ej, descSize);
			}
		}
	}
	return i + 1;
}

DWORD find_pci()
{
	// Config Address
	DWORD cmdAhci = 0;
	DWORD cmd = 0x80000000;
	while (cmd < 0x81000000)
	{
		__outdword(0x0CF8, cmd);
		DWORD id = __indword(0x0CFC);
		WORD vendor = id & 0xFFFF;
		if (!vendor || (vendor == 0xFFFF))
		{
			cmd += 0x800;
			continue;
		}

		// Read class code
		__outdword(0x0CF8, cmd + 0x08);
		DWORD classId = __indword(0x0CFC) >> 8;

		OutputAddressX(cmd, 8);
		OutputText(": ");
		OutputAddressX(id, 8);
		OutputText(" - ");
		OutputAddressX(classId, 6);
		OutputText("\r\n");

		if (classId == 0x010601) // AHCI
		{
			cmdAhci = cmd;
			OutputText("AHCI CONTROLLER FOUND\r\n");
			break;
		}

		cmd += 0x800;
	}
	if (!cmdAhci)
		return 0;

	void *iobase = pci_enable_mmio(cmdAhci, PCI_BASE_ADDRESS_5);
	if (!iobase)
	{
		OutputText("AHCI CONTROLLER MMIO NOT SUPPORTED\r\n");
		return 0;
	}
	OutputText("AHCI CONTROLLER MMIO ADDRESS ");
	OutputAddress((QWORD) iobase);
	OutputText("\r\n");

	DWORD dvc = cmdAhci;
	// Enable busmaster
	// Read Command register and set MASTER bit
	__outdword(0x0CF8, dvc + PCI_COMMAND);
	WORD val = __inword(0x0CFC) | PCI_COMMAND_MASTER;
	// Write to Command register
	__outdword(0x0CF8, dvc + PCI_COMMAND);
	__outword(0x0CFC, val);
	return 1;
}

unsigned long long EFIMainCRTStartup(void *handle, EFI_SYSTEM_TABLE *systemTable)
{
	SYSTEM_TABLE = systemTable;
	SYSTEM_TABLE->BootServices->SetWatchdogTimer(0, 0, 0, 0);

	// ENABLE SSE
	__writecr0((__readcr0() & (~4ULL)) | 2); // CLEAR CR0.EM AND SET CR0.MP
	__writecr4(__readcr4() | (3 << 9)); // SET CR4.OSFXSR AND CR4.OSXMMEEXCPT

	EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics = 0;
	SYSTEM_TABLE->BootServices->LocateProtocol(EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID, 0, &graphics);
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION info = DetectingMode(graphics);
	QWORD fbb = graphics->Mode->FrameBufferBase;

	/*
	for (DWORD i = 0; i < info.VerticalResolution; i++)
	{
		memset64((void *) fbb, 0x00FFFFFF00FFFFFFULL, info.HorizontalResolution >> 1);
		fbb += info.PixelsPerScanLine * 4;
	}
	*/
	QWORD buf[2] = { 0x00FFFFFF00FFFFFFULL, 0x00FFFFFF00FFFFFFULL };
	__memset128((void *) fbb, buf, (info.PixelsPerScanLine * info.VerticalResolution * 4) / 16);


	OutputText("Supernova-EFI\r\n");

	OutputText("GOP: ");
	OutputAddress(fbb);
	OutputText("\r\n");

	OutputText("IMAGE BASE ");
	OutputAddress((QWORD) &__ImageBase);
	OutputText("\r\n");

	DWORD cpuid[4] = {0, 0, 0, 0};
	__cpuid(cpuid, 0x80000001);
	DWORD edx = cpuid[3];
	DWORD _1gbpage = edx & (1 << 26) ? 0 : 1;
	if (_1gbpage)
		OutputText("CPU NOT SUPPORTED 1GB PAGING\r\n");

	char brand[51];
	brand[48] = '\r';
	brand[49] = '\n';
	brand[50] = 0;
	DWORD *brandx = (DWORD *) brand;
	__cpuid(cpuid, 0x80000002);
	brandx[0] = cpuid[0];
	brandx[1] = cpuid[1];
	brandx[2] = cpuid[2];
	brandx[3] = cpuid[3];
	__cpuid(cpuid, 0x80000003);
	brandx[4] = cpuid[0];
	brandx[5] = cpuid[1];
	brandx[6] = cpuid[2];
	brandx[7] = cpuid[3];
	__cpuid(cpuid, 0x80000004);
	brandx[8] = cpuid[0];
	brandx[9] = cpuid[1];
	brandx[10] = cpuid[2];
	brandx[11] = cpuid[3];
	OutputText("CPU: ");
	OutputText(brand);
	OutputText("\r\n");

	find_pci();


	jmp(0, 0);

	QWORD memMapSize = 0;
	EFI_MEMORY_DESCRIPTOR *MemoryMap = 0;
	QWORD MapKey = 0;
	QWORD MapDescSize = 0;
	DWORD MapVersion = 0;
	SYSTEM_TABLE->BootServices->GetMemoryMap(&memMapSize, MemoryMap, &MapKey, &MapDescSize, &MapVersion);

	QWORD totalSize = memMapSize + sizeof(EFI_MEMORY_DESCRIPTOR) * 20;
	SYSTEM_TABLE->BootServices->AllocatePool(EfiRuntimeServicesData, totalSize, &MemoryMap);
	memset(MemoryMap, 0, totalSize);
	QWORD ret = SYSTEM_TABLE->BootServices->GetMemoryMap(&totalSize, MemoryMap, &MapKey, &MapDescSize, &MapVersion);
	if (ret)
	{
		OutputText("CANNOT GET MEMORY MAP\r\n");
		SYSTEM_TABLE->BootServices->FreePool(MemoryMap);
		while (1) __halt();
	}


	QWORD entries = totalSize / MapDescSize;
	OutputNumber(entries);
	OutputText(" Memory Map\r\n");
	QWORD MMA = (QWORD) MemoryMap;
	SortMemoryMap(MMA, entries, MapDescSize);
	entries = CompactMemoryMap(MMA, entries, MapDescSize);
	MEMORY_REGION **end = (MEMORY_REGION **) 0x3018;
	MEMORY_REGION *beg = (MEMORY_REGION *) 0x3020;
	memset(beg, 0, sizeof(MEMORY_REGION));
	beg->F = 0xFF;
	// OUTPUTTEXT("Base Address       Length             Type\n");
	for (QWORD i = 0; i < entries; i++)
	{
		MEMORY_REGION region;
		EFI_MEMORY_DESCRIPTOR *memory = (EFI_MEMORY_DESCRIPTOR *) MMA;
		/*
		PRINTRAX(memory->PhysicalStart, 16);
		OUTPUTTEXT(" | ");
		PRINTRAX(memory->NumberOfPages << 12, 16);
		OUTPUTTEXT(" | ");
		OUTPUTTEXT(MEMORY_TYPE + 27 * memory->Type);
		LINEFEED();
		*/
		//OutputMemoryMap(memory);
		//LINEFEED();
		MMA += MapDescSize;
		region.A = memory->PhysicalStart;
		region.L = memory->NumberOfPages * 4096;
		region.X = 0;
		switch (memory->Type)
		{
			case EfiReservedMemoryType:
			case EfiMemoryMappedIO:
			case EfiMemoryMappedIOPortSpace:
				region.F = 2;// BIOS Reserved
				break;
			case EfiACPIReclaimMemory:
				region.F = 3;// BIOS ACPI Reclaimable Memory
				break;
			case EfiACPIMemoryNVS:
				region.F = 4;// BIOS ACPI NVS Memory
				break;
			case EfiUnusableMemory:
				region.F = 5;// BIOS BAD Memory
				break;
			default:
				region.F = 1;// Free
				break;
		}
		if (beg->F == 0xFF || (beg->F == region.F && beg->A + beg->L == region.A))
		{
			if (beg->F == 0xFF)
			{
				beg->F = region.F;
			}
			beg->L += region.L;
		}
		else
		{
			beg++;
			memcpy(beg, &region, sizeof(MEMORY_REGION));
		}
	}
	beg++;
	*end = beg;
	beg = (MEMORY_REGION *) 0x3020;
	OutputText("Base Address       Length             Type\r\n");
	//          0000000000000000 | 0000000000000000 | 00000000
	while (beg < *end)
	{
		OutputAddress(beg->A);
		OutputText(" | ");
		OutputAddress(beg->L);
		OutputText(" | ");
		OutputAddress(beg->F);
		OutputText("\r\n");
		beg++;
	}
	SYSTEM_TABLE->BootServices->ExitBootServices(handle, MapKey);


	jmp(0, 0);
	return 0;
}