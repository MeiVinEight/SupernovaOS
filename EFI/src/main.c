
#include "uefi.h"
#include "intrinsic.h"
#include "ntfs.h"

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

#define PA_PRESENT  0x001
#define PA_WRITE    0x002
#define PA_USER     0x004
#define PA_PAGESIZE 0x080

typedef struct _SUPERNOVA_BOOT_TABLE
{
	QWORD GUID0[2];
	QWORD GUID1[2];
} SUPERNOVA_BOOT_TABLE;
typedef struct _MEMORY_REGION
{
	QWORD A;
	QWORD L:56;
	QWORD F:8;
} MEMORY_REGION;
typedef struct _SUPERNOVA_SYSTEM_TABLE
{
	BYTE DOSHEAD[0x40];
	QWORD GUID0[2];
	QWORD GUID1[2];
	WORD HRES;
	WORD VRES;
	DWORD PPL;
	QWORD FBB;
	QWORD GDT[3];
	QWORD RSDP;
	BYTE RSV[0x970];
	DWORD DVC[0x80];
	MEMORY_REGION MEMORY[64];
	QWORD PAGE[];
} SUPERNOVA_SYSTEM_TABLE;

extern char __ImageBase;
UEFIAPI QWORD EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID[2] = { 0x4A3823DC9042A9DE, 0x6A5180D0DE7AFB96 };
UEFIAPI QWORD EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID[2] = { 0x11d26459964e5b22, 0x3b7269c9a000398e };
UEFIAPI QWORD EFI_FILE_INFO_GUID[2] = { 0x11d26d3f09576e92, 0x3b7269c9a000398E };
UEFIAPI QWORD EFI_ACPI_20_TABLE_GUID[2] = { 0x11D3E4F18868E871, 0x81883CC7800022BC };
UEFIAPI QWORD EFI_BLOCK_IO_PROTOCOL_GUID[2] = { 0x11D26459964E5B21, 0x3b7269c9a000398e };
//UEFIAPI __declspec(align(4096)) BYTE paging[4096];
UEFIAPI SUPERNOVA_BOOT_TABLE *BOOT_TABLE = (SUPERNOVA_BOOT_TABLE *) ((&__ImageBase) + 0x40);
UEFIAPI BYTE MFT_RECORD[1024];
UEFIAPI BYTE CORE_MFT_BUF[1024];
/*
UEFIAPI const char MEMORY_TYPE[] =
"EfiReservedMemoryType     \0"
"EfiLoaderCode             \0"
"EfiLoaderData             \0"
"EfiBootServicesCode       \0"
"EfiBootServicesData       \0"
"EfiRuntimeServicesCode    \0"
"EfiRuntimeServicesData    \0"
"EfiConventionalMemory     \0"
"EfiUnusableMemory         \0"
"EfiACPIReclaimMemory      \0"
"EfiACPIMemoryNVS          \0"
"EfiMemoryMappedIO         \0"
"EfiMemoryMappedIOPortSpace\0"
"EfiPalCode                \0"
"EfiPersistentMemory       \0"
"EfiUnacceptedMemoryType   \0"
"EfiMaxMemoryType          ";
*/

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
QWORD core_mapping(QWORD x)
{
	return x;
}
QWORD jmp(QWORD(*entry)(), QWORD stackAddr)
{
	if(!entry)
		while (1) __halt();

	BYTE call[24] = {
		0x48, 0xBC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // MOV RSP, stackAddr
		0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,                         // JMP [RIP+0x00000000]
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             // entry (8 bytes)
	};
	*((QWORD *) (call + 2)) = stackAddr;
	*((QWORD *) (call + 16)) = (QWORD) entry;
	return ((QWORD (*)()) call)();
}

void BlockIORead(EFI_BLOCK_IO_PROTOCOL *proto, QWORD lba, QWORD scnt, QWORD buffer)
{
	DWORD stat;
	if ((stat = proto->ReadBlocks(proto, proto->Meida->MediaId, lba, scnt << 9, (void *) buffer)))
	{
		OutputText("DISK READ ERROR: ");
		OutputNumber(stat);
		jmp(0, 0);
	}
}
void DetectingMode(EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics)
{
	int defaultMode = graphics->Mode->Mode;
	QWORD sqr = graphics->Mode->Info->HorizontalResolution * graphics->Mode->Info->VerticalResolution;
	int usedMode = defaultMode;
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
		}
	}
	graphics->SetMode(graphics, usedMode);
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
			__memcpy(desc, (void *) (map + i * descSize), descSize);
			__memcpy((void *) (map + i * descSize), minEntry, descSize);
			__memcpy(minEntry, desc, descSize);
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
				__memcpy(ei, ej, descSize);
			}
		}
	}
	return i + 1;
}
DWORD ntfs_read_mft_record(EFI_BLOCK_IO_PROTOCOL *proto, NTFS_BPB *bpb, NTFS_MFT_ATTR_HEADER *attr, DWORD mftNum, QWORD bufAddr)
{
	DWORD mftPreClus = bpb->cluster >> 1;
	DWORD vcn = mftNum / mftPreClus;
	DWORD lcn = ntfs_logical_cluster_number(attr, vcn);
	DWORD mftInClus = mftNum % mftPreClus;
	QWORD sector = bpb->hidden;
	sector += lcn * bpb->cluster;
	sector += (QWORD) mftInClus << 1;
	BlockIORead(proto, sector, 2, bufAddr);
	*((WORD *) (bufAddr + 0x03FE)) = *((WORD *) (bufAddr + 0x34));
	*((WORD *) (bufAddr + 0x01FE)) = *((WORD *) (bufAddr + 0x32));
	return 1;
}
DWORD loading_core_sata(EFI_BLOCK_IO_PROTOCOL *port, QWORD page)
{
	// Read LBA 1: GPT Header
	BlockIORead(port, 1, 1, page);
	QWORD efipart = 0x5452415020494645ULL; // "EFI PART"
	if (efipart != *((QWORD *) page))
		return 0;

	// Assume GPT and check DISK GUID
	QWORD *guid = (QWORD *) (page + 0x38);
	if (guid[0] != BOOT_TABLE->GUID0[0] && guid[1] != BOOT_TABLE->GUID0[1])
	{
		return 0;
	}

	// FIND NTFS SYSTEM PART
	// Read partition table
	BlockIORead(port, *((QWORD *) (page + 0x48)), 1, page);

	__memset((void *) (page + 0x200), 0, 0x200);
	QWORD partEntry = page;
	for (; partEntry < page + 0x200; partEntry += 0x80)
	{
		QWORD *partGUID = (QWORD *) (partEntry + 0x10);
		if (!(partGUID[0] | partGUID[1]))
			return 0;

		if (partGUID[0] == BOOT_TABLE->GUID1[0] && partGUID[1] == BOOT_TABLE->GUID1[1])
			break;
	}

	OutputText("PARTITION FOUND\r\n");

	QWORD partLBA = *((QWORD *) (partEntry + 0x20));
	BlockIORead(port, partLBA, 1, page);

	NTFS_BPB ntfsBpb;
	__memcpy(&ntfsBpb, (void *) page, sizeof(NTFS_BPB));

	// $MFT LBA
	QWORD mftsector = ntfsBpb.hidden + (ntfsBpb.MFT * ntfsBpb.cluster);
	OutputText("$MFT @ ");
	OutputNumber(mftsector);
	OutputText("\r\n");
	BlockIORead(port, mftsector, 2, page);
	*((WORD *) (page + 0x03FE)) = *((WORD *) (page + 0x34));
	*((WORD *) (page + 0x01FE)) = *((WORD *) (page + 0x32));
	__memcpy(MFT_RECORD, (void *) page, 1024);

	NTFS_MFT_ATTR_HEADER *attr = ntfs_attr_data(MFT_RECORD);
	if (!attr)
		return 0;

	//logical_cluster_number((NTFS_MFT_ATTR_HEADER *) attribute, 0xFFFF);

	DWORD mftNum = 0;
	char fileName[256];
	char *fileNameList[] = {"Supernova", "SYSTEM", "CORE.DLL"};
	DWORD coreMFT = 0;

	DWORD mftCount = attr->nonResident.dataSize / 1024;
	for (; mftNum < mftCount; mftNum++)
	{
		DWORD nowMFTNum = mftNum;
		for (int i = 3; i--;)
		{
			ntfs_read_mft_record(port, &ntfsBpb, attr, nowMFTNum, page);
			NTFS_MFT_ATTR_FILE_NAME *fnAttr = ntfs_attr_file_name((void *) page);
			if (!fnAttr)
				goto NEXT_MFT;
			DWORD fileNameLen = ntfs_file_name(fnAttr, fileName);
			if (!fileNameLen)
				goto NEXT_MFT;
			if (__memcmp(fileName, fileNameList[i], fileNameLen))
				goto NEXT_MFT;

			//break;
			nowMFTNum = fnAttr->parentDirMFTNum & 0xFFFFFFFF;
		}
		if (nowMFTNum == 5 && mftNum != 5)
		{
			coreMFT = mftNum;
			break;
		}

		NEXT_MFT:;
	}
	if (!coreMFT)
		return 0;

	OutputText("CORE.DLL @ ");
	OutputNumber(coreMFT);
	OutputText("\r\n");

	ntfs_read_mft_record(port, &ntfsBpb, attr, coreMFT, page);

	NTFS_MFT_ATTR_HEADER *dataAttr = ntfs_attr_data((void *) page);
	__memcpy(CORE_MFT_BUF, dataAttr, dataAttr->length);
	dataAttr = (NTFS_MFT_ATTR_HEADER *) CORE_MFT_BUF;
	QWORD bufAddr = 0;

	OutputText("CORE.DLL cluster ");
	OutputNumber(dataAttr->nonResident.startVCN);
	OutputText(" - ");
	OutputNumber(dataAttr->nonResident.lastVCN);
	OutputText("\r\n");
	for (DWORD vcn = dataAttr->nonResident.startVCN; vcn <= dataAttr->nonResident.lastVCN; vcn++)
	{
		QWORD lcn = ntfs_logical_cluster_number(dataAttr, vcn);
		if (!lcn)
			return 0;
		QWORD sector = ntfsBpb.hidden + lcn * ntfsBpb.cluster;
		BlockIORead(port, sector, ntfsBpb.cluster, page);
		__memcpy((void *) bufAddr, (void *) page, (QWORD) ntfsBpb.cluster << 9);
		bufAddr += 512 * ntfsBpb.cluster;
	}


	return 1;
}
DWORD DetectingDisk(EFI_SYSTEM_TABLE *table)
{
	void **handleBuffer = 0;
	QWORD handleNum = 0;
	DWORD stat = table->BootServices->LocateHandleBuffer(ByProtocol, EFI_BLOCK_IO_PROTOCOL_GUID, 0, &handleNum, (void **) &handleBuffer);
	if (stat)
	{
		OutputText("Cannot locate handle\r\n");
		return 0;
	}

	OutputText("Locate ");
	OutputNumber(handleNum);
	OutputText(" Handles\r\n");

	if (handleNum > 256)
		handleNum = 256;

	QWORD *pageBuf = (QWORD *) 0x1000;
	stat = SYSTEM_TABLE->BootServices->AllocatePages(AllocateAnyPages, EfiBootServicesData, 1, (void *) &pageBuf);
	if (stat)
	{
		OutputText("Cannot allocate\r\n");
		return 0;
	}
	EFI_BLOCK_IO_PROTOCOL *proto = 0;
	DWORD finding = 0;
	for (QWORD i = 0; i < handleNum; i++)
	{
		SYSTEM_TABLE->BootServices->HandleProtocol(handleBuffer[i], EFI_BLOCK_IO_PROTOCOL_GUID, (void **) &proto);

		if (proto->Meida->LogicalPartition)
			continue;

		OutputAddress((QWORD) handleBuffer[i]);
		OutputText(" ");
		OutputNumber(proto->Meida->MediaId);
		OutputText(": ");
		OutputNumber(proto->Meida->LogicalPartition);
		OutputText("\r\n");

		proto->ReadBlocks(proto, proto->Meida->MediaId, 1, 512, pageBuf);
		OutputText((char *) pageBuf);
		OutputText("\r\n");

		if ((finding = loading_core_sata(proto, (QWORD) pageBuf)))
			break;
	}
	SYSTEM_TABLE->BootServices->FreePages((QWORD) pageBuf, 1);
	SYSTEM_TABLE->BootServices->FreePool(handleBuffer);
	return finding;
}

unsigned long long EFIMainCRTStartup(void *handle, EFI_SYSTEM_TABLE *systemTable)
{
	SYSTEM_TABLE = systemTable;
	SYSTEM_TABLE->BootServices->SetWatchdogTimer(0, 0, 0, 0);

	// ENABLE SSE
	__writecr0((__readcr0() & (~4ULL)) | 2); // CLEAR CR0.EM AND SET CR0.MP
	__writecr4(__readcr4() | (3 << 9)); // SET CR4.OSFXSR AND CR4.OSXMMEEXCPT

	int maxMode = 0;
	QWORD chars = 0;
	for (DWORD i = 0; i < SYSTEM_TABLE->ConOut->Mode->MaxMode; i++)
	{
		QWORD cols, rows;
		SYSTEM_TABLE->ConOut->QueryMode(SYSTEM_TABLE->ConOut, i, &cols, &rows);
		if (cols * rows > chars)
		{
			chars = cols * rows;
			maxMode = i;
		}
	}
	SYSTEM_TABLE->ConOut->SetMode(SYSTEM_TABLE->ConOut, maxMode);

	EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics = 0;
	SYSTEM_TABLE->BootServices->LocateProtocol(EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID, 0, (void **) &graphics);
	DetectingMode(graphics);
	QWORD fbb = graphics->Mode->FrameBufferBase;

	/*
	for (DWORD i = 0; i < info.VerticalResolution; i++)
	{
		memset64((void *) fbb, 0x00FFFFFF00FFFFFFULL, info.HorizontalResolution >> 1);
		fbb += info.PixelsPerScanLine * 4;
	}
	*/
	QWORD buf[2] = { 0x00FFFFFF00FFFFFFULL, 0x00FFFFFF00FFFFFFULL };
	//__memset128((void *) fbb, buf, (info.PixelsPerScanLine * info.VerticalResolution * 4) / 16);


	OutputText("Supernova-EFI @ ");
	OutputAddress((QWORD) &__ImageBase);
	OutputText("\r\n");

	int cpuid[4] = {0, 0, 0, 0};
	char brand[51];
	brand[48] = '\r';
	brand[49] = '\n';
	brand[50] = 0;
	DWORD *brandx = (DWORD *) brand;
	__cpuid(cpuid, (int) 0x80000002);
	brandx[0] = cpuid[0];
	brandx[1] = cpuid[1];
	brandx[2] = cpuid[2];
	brandx[3] = cpuid[3];
	__cpuid(cpuid, (int) 0x80000003);
	brandx[4] = cpuid[0];
	brandx[5] = cpuid[1];
	brandx[6] = cpuid[2];
	brandx[7] = cpuid[3];
	__cpuid(cpuid, (int) 0x80000004);
	brandx[8] = cpuid[0];
	brandx[9] = cpuid[1];
	brandx[10] = cpuid[2];
	brandx[11] = cpuid[3];
	OutputText("CPU: ");
	OutputText(brand);
	OutputText("\r\n");

	OutputText("Video Memory @ ");
	OutputAddress(fbb);
	OutputText("\r\n");
	OutputText("Video ");
	OutputNumber(graphics->Mode->Info->HorizontalResolution);
	OutputText("x");
	OutputNumber(graphics->Mode->Info->VerticalResolution);
	OutputText("\r\n");

	QWORD rsdp = 0;
	for (QWORD i = 0; i < SYSTEM_TABLE->NumberOfTableEntries; i++)
	{
		if (
			SYSTEM_TABLE->ConfigurationTable[i].GUID[0] == EFI_ACPI_20_TABLE_GUID[0] &&
			SYSTEM_TABLE->ConfigurationTable[i].GUID[1] == EFI_ACPI_20_TABLE_GUID[1]
			)
		{
			rsdp = (QWORD) SYSTEM_TABLE->ConfigurationTable[i].TABLE;
			OutputText("ACPI 2.0 RSD PTR @ ");
			OutputAddress(rsdp);
			OutputText("\r\n");
			break;
		}
	}
	if (!DetectingDisk(systemTable))
	{
		OutputText("CORE.DLL NOT FOUND");
		jmp(0, 0);
	}

	QWORD memMapSize = 0;
	EFI_MEMORY_DESCRIPTOR *MemoryMap = 0;
	QWORD MapKey = 0;
	QWORD MapDescSize = 0;
	DWORD MapVersion = 0;
	SYSTEM_TABLE->BootServices->GetMemoryMap(&memMapSize, MemoryMap, &MapKey, &MapDescSize, &MapVersion);

	QWORD totalSize = memMapSize + sizeof(EFI_MEMORY_DESCRIPTOR) * 20;
	SYSTEM_TABLE->BootServices->AllocatePool(EfiRuntimeServicesData, totalSize, (void **) &MemoryMap);
	__memset(MemoryMap, 0, totalSize);
	QWORD ret = SYSTEM_TABLE->BootServices->GetMemoryMap(&totalSize, MemoryMap, &MapKey, &MapDescSize, &MapVersion);
	if (ret)
	{
		OutputText("CANNOT GET MEMORY MAP\r\n");
		SYSTEM_TABLE->BootServices->FreePool(MemoryMap);
		while (1) __halt();
	}

	SUPERNOVA_SYSTEM_TABLE *SST = 0;
	__assume(SST != 0);
	QWORD entries = totalSize / MapDescSize;
	//OutputNumber(entries);
	//OutputText(" Memory Map\r\n");
	QWORD MMA = (QWORD) MemoryMap;
	SortMemoryMap(MMA, entries, MapDescSize);
	entries = CompactMemoryMap(MMA, entries, MapDescSize);
	//MEMORY_REGION **end = (MEMORY_REGION **) 0x3018;
	MEMORY_REGION *beg = SST->MEMORY;
	__memset(beg, 0, sizeof(MEMORY_REGION));
	beg->F = 0xFF;
	// OUTPUTTEXT("Base Address       Length             Type\n");
	for (QWORD i = 0; i < entries; i++)
	{
		MEMORY_REGION region;
		EFI_MEMORY_DESCRIPTOR *memory = (EFI_MEMORY_DESCRIPTOR *) MMA;
		/*
		OutputAddress(memory->PhysicalStart);
		OutputText(" | ");
		OutputAddress(memory->NumberOfPages << 12);
		OutputText(" | ");
		OutputText(MEMORY_TYPE + 27 * memory->Type);
		OutputText("\r\n");
		*/
		MMA += MapDescSize;
		region.A = memory->PhysicalStart;
		region.L = memory->NumberOfPages * 4096;
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
		if (region.A < 0x10000)
			region.F = 1;// Force to free if below 64KB
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
			__memcpy(beg, &region, sizeof(MEMORY_REGION));
		}
	}
	beg++;
	beg->A = -1ULL;
	//*end = beg;
	/*
	beg = SST->MEMORY;
	//          0000000000000000 | 0000000000000000 | 00000000

	OutputText("Base Address       Length             Type\r\n");
	while (~beg->A)
	{
		OutputAddress(beg->A);
		OutputText(" | ");
		OutputAddress(beg->L);
		OutputText(" | ");
		OutputAddress(beg->F);
		OutputText("\r\n");
		beg++;
	}
	*/
	SYSTEM_TABLE->BootServices->ExitBootServices(handle, MapKey);
	__cli();
	char idtr[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	__lidt(idtr);

	SST->GUID0[0] = BOOT_TABLE->GUID0[0];
	SST->GUID0[1] = BOOT_TABLE->GUID0[1];
	SST->GUID1[0] = BOOT_TABLE->GUID1[0];
	SST->GUID1[1] = BOOT_TABLE->GUID1[1];
	SST->HRES = graphics->Mode->Info->HorizontalResolution;
	SST->VRES = graphics->Mode->Info->VerticalResolution;
	SST->PPL = graphics->Mode->Info->PixelsPerScanLine;
	SST->FBB = graphics->Mode->FrameBufferBase;
	SST->RSDP = rsdp;

	// Setup paging
	QWORD *paging = SST->PAGE;
	//paging_attribute((QWORD) paging, PA_WRITE);
	paging[0x000] = ((QWORD) (paging + 0x200)) | 3;
	paging[0x100] = ((QWORD) (paging + 0x200)) | 3;
	paging += 0x200;
	paging[0] = ((QWORD) (paging + 0x200)) | 3;
	paging[1] = ((QWORD) (paging + 0x400)) | 3;
	paging[2] = ((QWORD) (paging + 0x600)) | 3;
	paging[3] = ((QWORD) (paging + 0x800)) | 3;
	paging += 0x200;
	QWORD memAddr = 0;
	for (QWORD i = 0; i < 4; i++)
	{
		for (QWORD j = 0; j < 512; j++)
		{
			paging[(i * 0x200) + j] = memAddr | PA_PRESENT | PA_WRITE | PA_PAGESIZE;
			memAddr += 0x00200000;
		}
	}
	__writecr3((QWORD) SST->PAGE);



	QWORD stackPoint = SST->MEMORY[0].L - 0x40;
	QWORD base = 0;
	__assume (base != 0);
	QWORD ntHeader = *((DWORD *) (base + 0x3C));
	QWORD entry = *((DWORD *) (ntHeader + 0x28));
	stackPoint |= 0xFFFF800000000000ULL;
	entry |= 0xFFFF800000000000ULL;


	jmp((QWORD(*)()) entry, stackPoint);
	//jmp(0, 0);
	return 0;
}