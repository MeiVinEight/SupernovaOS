#include <driver/nvme/nvme.h>
#include <mm/vmm.h>
#include <driver/pci/pci.h>
#include <core.h>
#include <stdio.h>
#include <timer/timer.h>

PCI_EXPRESS_NVME_CONTROLLER *NVME_CONTROLLER;

WORD nvm_express_submission(NVM_EXPRESS_SUBMISSION_QUEUE *queue, void *sub)
{
	++queue->COID;
	if (queue->COID == 0xFFFF || !queue->COID)
		queue->COID = 1;
	WORD coid = queue->COID;
	queue->RING[queue->INDX] = *((NVM_EXPRESS_SUBMISSION *) sub);
	queue->RING[queue->INDX].COID = coid;
	if (queue->INDX >= queue->SIZE)
		queue->INDX = 0;
	else
		queue->INDX++;
	*queue->BELL = queue->INDX;
	return coid;
}
WORD nvm_express_completion(NVM_EXPRESS_COMPLETION_QUEUE *queue, NVM_EXPRESS_COMPLETION *com)
{
	for (; 1; delay(1))
	{
		if (queue->RING[queue->INDX].PHAS != queue->CYCL)
			continue;

		NVM_EXPRESS_COMPLETION comp = queue->RING[queue->INDX];
		if (com)
			*com = comp;
		if (queue->INDX == queue->SIZE)
		{
			queue->INDX = 0;
			queue->CYCL ^= 1;
		}
		else queue->INDX++;
		*queue->BELL = queue->INDX;
		return comp.STAT;
	}
}
QWORD nvm_express_transfer(NVM_EXPRESS_STORAGE *nvme, DWORD namespace, QWORD lba, void *buf, DWORD count, DWORD io)
{
	QWORD phyAddr = physical_address((QWORD) buf);
	QWORD page0 = phyAddr &= ~0xFFFULL;
	QWORD page1 = (phyAddr + ((QWORD) count << 9)) & ~0xFFFULL;
	if (page0 != page1)
		return 1;

	NVM_EXPRESS_SUBMISSION_READ read;
	NVM_EXPRESS_COMPLETION comp;
	__memset(&comp, 0, sizeof(comp));
	__memset(&read, 0, sizeof(NVM_EXPRESS_SUBMISSION_READ));
	read.OPCO = NVME_CMD_WRITE + io;
	read.PRP1 = phyAddr;
	read.NSID = namespace;
	read.LBA0 = lba;
	read.CONT = count;
	WORD coid = nvm_express_submission(&nvme->CTRL->IOSQ, &read);
	DWORD cc = nvm_express_completion(&nvme->CTRL->IOCQ, &comp);
	if (coid != comp.COID)
		return 2;
	if (cc)
	{
		nvme->STAT = cc;
		return 3;
	}
	return 0;
}
QWORD nvm_express_read(STANDARD_STORAGE_DEVICE *ssd, void *buf, QWORD lba, DWORD count)
{
	NVM_EXPRESS_STORAGE *nvme = (NVM_EXPRESS_STORAGE *) ssd;
	QWORD addr = (QWORD) buf;
	if (addr & 0xFFF)
	{
		BYTE *bufx = buf;
		QWORD lbax = lba;
		DWORD cont = count;
		BYTE *alig = nvme->BUFF;
		while (cont)
		{
			DWORD rc = 8;
			if (rc > cont)
				rc = cont;
			QWORD cc = nvm_express_transfer(nvme, nvme->NSID, lbax, alig, cont, NVME_TRANSFER_READ);
			if (cc)
				return 1;
			__memcpy(bufx, alig, (QWORD) rc << 9);
			bufx += (QWORD) rc << 9;
			lbax += rc;
			cont -= rc;
		}
	}
	else
	{
		BYTE *bufx = buf;
		QWORD lbax = lba;
		DWORD cont = count;
		while (cont)
		{
			DWORD rc = 8;
			if (rc > cont)
				rc = cont;
			(void) *((volatile DWORD *) bufx);
			QWORD cc = nvm_express_transfer(nvme, nvme->NSID, lbax, bufx, cont, NVME_TRANSFER_READ);
			if (cc)
				return 1;
			bufx += (QWORD) rc << 9;
			lbax += rc;
			cont -= rc;
		}
	}
	return 0;
}
void nvme_controller_setup(PCI_EXPRESS_DEVICE *pcie)
{
	PCI_DEVICE_VENDOR vendor;
	vendor.VENDOR = pcie->configuration->vendor;
	vendor.DEVICE = pcie->configuration->device;
	printf("PCI Express @ %016llX: %06X - ", (QWORD) pcie->configuration, pcie->configuration->class);
	const char *vendorName = pci_vendor_name(vendor.VENDOR);
	const char *deviceName = pci_device_name(vendor);
	if (vendorName && deviceName)
		printf("%s %s\n", vendorName, deviceName);
	else
		printf("%08lX\n", vendor.ID);

	PCI_EXPRESS_NVME_CONTROLLER *controller = heap_alloc(sizeof(PCI_EXPRESS_NVME_CONTROLLER));
	__memset(controller, 0, sizeof(PCI_EXPRESS_NVME_CONTROLLER));
	controller->PCIE = *pcie;
	controller->NEXT = NVME_CONTROLLER;
	NVME_CONTROLLER = controller;
	controller->NVME = (NVM_EXPRESS_CONTROLLER *) core_mapping(pcie_cfg_get_base_address(&controller->PCIE,  0));
	controller->CAPA = controller->NVME->CAPA;

	// 1. The host waits for the controller to indicate that any previous reset is complete by waiting for
	//    CSTS.RDY to become ‘0’;
	DWORD waitTime = 0;
	while (1)
	{
		if (waitTime > 5000)
		{
			printf("NVM Express Time out waiting for ready!\n");
			return;
		}

		if (controller->NVME->CFST)
		{
			printf("NVM Express Controller Fatal status detected!\n");
			return;
		}

		if (controller->NVME->RADY != controller->NVME->ENAB)
			goto WAITING;

		if (!controller->NVME->ENAB)
			break;

		controller->NVME->ENAB = 0;

		WAITING:;
		delay(1);
		waitTime++;
	}
	// 2. The host configures the Admin Queue by setting the Admin Queue Attributes (AQA), Admin
	//    Submission Queue Base Address (ASQ), and Admin Completion Queue Base Address (ACQ) to
	//    appropriate values;
	QWORD stride = 1ULL << controller->CAPA.DSTR;

	controller->CASQ.RING = (NVM_EXPRESS_SUBMISSION *) core_mapping(alloc_physical_memory(1, 0));
	__memset(controller->CASQ.RING, 0, 0x1000);
	controller->CASQ.BELL = controller->NVME->BELL;
	controller->CASQ.SIZE = (0x1000 / sizeof(NVM_EXPRESS_SUBMISSION)) - 1;
	controller->NVME->ASQS = (WORD) controller->CASQ.SIZE;
	controller->NVME->ASQA = physical_address((QWORD) controller->CASQ.RING);

	controller->CACQ.RING = (NVM_EXPRESS_COMPLETION *) core_mapping(alloc_physical_memory(1, 0));
	__memset(controller->CACQ.RING, 0, 0x1000);
	controller->CACQ.BELL = controller->NVME->BELL + stride;
	controller->CACQ.SIZE = (0x1000 / sizeof(NVM_EXPRESS_COMPLETION)) - 1;
	controller->CACQ.CYCL = 1;
	controller->NVME->ACQS = (WORD) controller->CACQ.SIZE;
	controller->NVME->ACQA = physical_address((QWORD) controller->CACQ.RING);
	//controller->NVME->CAQA = (((WORD) controller->CACQ.SIZE + 1) << 16) | (controller->CASQ.SIZE + 1);



	// 3. The host determines the supported I/O Command Sets by checking the state of CAP.CSS and
	//    appropriately initializing CC.CSS as follows:
	//    a. If the CAP.CSS.NOIOCSS bit is set to ‘1’, then the CC.CSS field should be set to 111b;
	//    b. If the CAP.CSS.IOCSS bit is set to ‘1’, then the CC.CSS field should be set to 110b; and
	//    c. If the CAP.CSS.IOCSS bit is cleared to ‘0’ and the CAP.CSS.NCSS bit is set to ‘1’, then the
	//       CC.CSS field should be set to 000b;
	/*
	if (controller->NVME->CAPA.CSSU & NVME_CAP_CSS_NOIOCSS)
		controller->NVME->ICSS = 0b111;
	else if (controller->NVME->CAPA.CSSU & NVME_CAP_CSS_IOCSS)
		controller->NVME->ICSS = 0b110;
	else if (controller->NVME->CAPA.CSSU & NVME_CAP_CSS_NCSS)
		controller->NVME->ICSS = 0;
	*/
	controller->NVME->ICSS = 0;

	// 4. The controller settings should be configured. Specifically:
	//    a. The arbitration mechanism should be selected in CC.AMS; and
	//    b. The memory page size should be initialized in CC.MPS;
	//controller->NVME->CONF = 0;
	controller->NVME->MPGS = controller->CAPA.MIPS;
	controller->NVME->AMSL = 0; // Round Robin
	controller->NVME->ISQE = 6; // 64b
	controller->NVME->ICQE = 4; // 16b

	// 5. The host enables the controller by setting CC.EN to ‘1’;
	controller->NVME->ENAB = 1;

	// 6. The host waits for the controller to indicate that the controller is ready to process commands. The
	//    controller is ready to process commands when CSTS.RDY is set to ‘1’;
	waitTime = 500;
	while (!(controller->NVME->STAT & 1) && waitTime)
	{
		if (controller->NVME->STAT & 2)
		{
			printf("NVM Express Controller Fatal status detected!\n");
			return;
		}
		delay(1);
		waitTime--;
	}
	if (!(controller->NVME->STAT & 1))
	{
		printf("NVM Express Enable Timeout!\n");
		return;
	}

	// 7. The host determines the configuration of the controller by issuing the Identify command specifying
	//    the Identify Controller data structure (i.e., CNS 01h);
	// Identify Controller
	QWORD phyAddr = alloc_physical_memory(1, 0);
	void *buf = (BYTE *) core_mapping(phyAddr);
	__memset(buf, 0, 0x1000);
	NVM_EXPRESS_SUBMISSION_IDENTIFY iden;
	__memset(&iden, 0, sizeof(iden));
	iden.OPCO = NVME_ADM_CMD_IDENTIFY;
	iden.PRP1 = phyAddr;
	iden.CNST = NVME_IDENTIFY_CNS_CONTROLLER; // CNS = 01h (Idneitfy Controller)
	WORD coid = nvm_express_submission(&controller->CASQ, &iden);

	NVM_EXPRESS_COMPLETION comp;
	nvm_express_completion(&controller->CACQ, &comp);
	if (comp.COID != coid)
		printf("Completion COID %d not equal submission COID %d\n", comp.COID, coid);

	NVM_EXPRESS_IDENTIFY_CONTROLLER *identify = buf;
	controller->NSCN = identify->NSCN;

	controller->IOSQ.RING = (NVM_EXPRESS_SUBMISSION *) identify;
	__memset(controller->IOSQ.RING, 0, 0x1000);
	controller->IOSQ.BELL = controller->NVME->BELL + (2 * stride);
	controller->IOSQ.SIZE = (0x1000 / sizeof(NVM_EXPRESS_SUBMISSION)) - 1;
	controller->IOCQ.RING = (NVM_EXPRESS_COMPLETION *) core_mapping(alloc_physical_memory(1, 0));
	__memset(controller->IOCQ.RING, 0, 0x1000);
	controller->IOCQ.BELL = controller->NVME->BELL + (3 * stride);
	controller->IOCQ.SIZE = (0x1000 / sizeof(NVM_EXPRESS_COMPLETION)) - 1;
	controller->IOCQ.CYCL = 1;

	// Create I/O Completion Queue
	NVM_EXPRESS_SUBMISSION_CREATE_CQ ccq;
	__memset(&ccq, 0, sizeof(ccq));
	ccq.OPCO = NVME_ADM_CMD_CREATE_CQ;
	ccq.PRP1 = physical_address((QWORD) controller->IOCQ.RING);
	ccq.QUID = 1;
	ccq.QUSZ = controller->IOCQ.SIZE;
	ccq.PCON = 1;
	coid = nvm_express_submission(&controller->CASQ, &ccq);
	__memset(&comp, 0, sizeof(comp));
	nvm_express_completion(&controller->CACQ, &comp);
	if (comp.COID != coid)
		printf("Completion COID %d not equal submission COID %d\n", comp.COID, coid);

	// Create I/O Submission Queue
	NVM_EXPRESS_SUBMISSION_CREATE_SQ csq;
	__memset(&csq, 0, sizeof(csq));
	csq.OPCO = NVME_ADM_CMD_CREATE_SQ;
	csq.PRP1 = physical_address((QWORD) controller->IOSQ.RING);
	csq.QUID = 1;
	csq.QUSZ = controller->IOSQ.SIZE;
	csq.PCON = 1;
	csq.CQID = 1;
	coid = nvm_express_submission(&controller->CASQ, &csq);
	__memset(&comp, 0, sizeof(comp));
	nvm_express_completion(&controller->CACQ, &comp);
	if (comp.COID != coid)
		printf("Completion COID %d not equal submission COID %d\n", comp.COID, coid);

	// Identify namespaces
	phyAddr = alloc_physical_memory(1, 0);
	for (DWORD nsid = 1; nsid <= controller->NSCN; nsid++)
	{
		NVM_EXPRESS_IDENTIFY_NAMESPACE *idns = (NVM_EXPRESS_IDENTIFY_NAMESPACE *) core_mapping(phyAddr);
		__memset(idns, 0, 0x1000);
		__memset(&iden, 0, sizeof(NVM_EXPRESS_SUBMISSION_IDENTIFY));
		iden.OPCO = NVME_ADM_CMD_IDENTIFY;
		iden.NSID = nsid;
		iden.PRP1 = phyAddr;
		iden.CNST = NVME_IDENTIFY_CNS_NAMESPACE;
		coid = nvm_express_submission(&controller->CASQ, &iden);
		__memset(&comp, 0, sizeof(comp));
		if (nvm_express_completion(&controller->CACQ, &comp))
			printf("NVM Express Namespace %lu Identify: %u\n", nsid, comp.STAT);
		if (comp.COID != coid)
			printf("Completion COID %d not equal submission COID %d\n", comp.COID, coid);
		if (!idns->NSZE)
			continue;

		BYTE flbas = idns->LBAS;
		BYTE lbafIdx = flbas & 0xF | ((flbas >> 5) & 0x3);
		NVM_EXPRESS_LBA_FORMAT *lbaf = idns->LBAF + lbafIdx;
		if (lbaf->LBAD != 9)
		{
			printf("NVM Express Namespace %lu: unsupported LBA size: %llu\n", nsid, 1ULL << lbaf->LBAD);
			continue;
		}

		NVM_EXPRESS_STORAGE *nvme = heap_alloc(sizeof(NVM_EXPRESS_STORAGE));
		__memset(nvme, 0, sizeof(NVM_EXPRESS_STORAGE));
		nvme->SSDV.READ = nvm_express_read;
		nvme->SSDV.CAPA = idns->NSZE;
		nvme->CTRL = controller;
		nvme->BUFF = (void *) core_mapping(alloc_physical_memory(1, 0));
		nvme->NSID = nsid;
		storage_insert(&nvme->SSDV);
	}
	free_physical_memory(phyAddr, 1);
}