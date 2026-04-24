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
	printf("NVM Express @ %p\n", controller->NVME);

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
	controller->NVME->ASQS = (WORD) controller->CASQ.SIZE + 1;
	controller->NVME->ASQA = physical_address((QWORD) controller->CASQ.RING);

	controller->CACQ.RING = (NVM_EXPRESS_COMPLETION *) core_mapping(alloc_physical_memory(1, 0));
	__memset(controller->CACQ.RING, 0, 0x1000);
	controller->CACQ.BELL = controller->NVME->BELL + stride;
	controller->CACQ.SIZE = (0x1000 / sizeof(NVM_EXPRESS_COMPLETION)) - 1;
	controller->CACQ.CYCL = 1;
	controller->NVME->ACQS = (WORD) controller->CACQ.SIZE + 1;
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
	controller->NVME->ENAB = 1;
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

	// Identify Controller
	QWORD phyAddr = alloc_physical_memory(1, 0);
	BYTE *buf = (BYTE *) core_mapping(phyAddr);
	__memset(buf, 0, 0x1000);
	NVM_EXPRESS_SUBMISSION_IDENTIFY sub;
	__memset(&sub, 0, sizeof(sub));
	sub.OPCO = NVME_ADM_CMD_IDENTIFY;
	sub.PRP1 = phyAddr;
	sub.CNST = NVME_IDENTIFY_CONTROLLER; // CNS = 01h (Idneitfy Controller)
	WORD coid = nvm_express_submission(&controller->CASQ, &sub);

	NVM_EXPRESS_COMPLETION comp;
	nvm_express_completion(&controller->CACQ, &comp);
	if (comp.COID != coid)
		printf("Completion COID %d not equal submission COID %d\n", comp.COID, coid);

	NVM_EXPRESS_IDENTIFY *identify = (NVM_EXPRESS_IDENTIFY *) buf;
	char name[65];
	__memcpy(name, identify->SERN, 20);
	name[20] = 0;
	printf("NVM Express serial: %s\n", name);
	__memcpy(name, identify->MODN, 40);
	name[40] = 0;
	printf("NVM Express model : %s\n", name);
}