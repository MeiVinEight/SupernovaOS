#include <driver/usb/usb_msc.h>
#include <driver/usb/usb_bulk.h>
#include <driver/xhci/xhci_context.h>
#include <stdio.h>
#include <intrinsic.h>
#include <driver/scsi/scsi_cmd.h>
#include <driver/scsi/scsi.h>
#include <mm/vmm.h>
#include <core.h>

DWORD scsi_command(XHCI_USB_DEVICE *device, DWORD iepid, DWORD oepid, DWORD isIn, DWORD lun, const void *cmd, DWORD cmdLen, void *data, DWORD dataLen)
{
	USB_MASS_STORAGE_BULK_CBW cbw;
	USB_MASS_STORAGE_BULK_CSW csw;
	__memset(&cbw, 0, sizeof(cbw));
	__memset(&csw, 0, sizeof(csw));
	cbw.SIGN = CBW_SIGNATURE;
	cbw.TAGX = 1;
	cbw.DTRL = dataLen;
	cbw.FLAG = (dataLen && isIn) ? USB_DIR_IN : USB_DIR_OUT;
	cbw.CLUN = lun;
	cbw.CLEN = cmdLen;
	__memcpy(cbw.CMMD, cmd, cmdLen);
	DWORD cc;
	if ((cc = xhci_transfer(device, oepid, 1, 0, &cbw, 31)) != XHCI_CODE_SUCCESS)
	{
		printf("USB SCSI Transfer CBW failed: %lu\n", cc);
		return cc;
	}
	if (dataLen)
	{
		if ((cc = xhci_transfer(device, (isIn ? iepid : oepid), 1, 0, data, dataLen)) != XHCI_CODE_SUCCESS)
		{
			printf("USB SCSI Transfer Data failed: %lu\n", cc);
			return cc;
		}
	}
	if ((cc = xhci_transfer(device, iepid, 1, 0, &csw, 13)) != XHCI_CODE_SUCCESS)
	{
		printf("USB SCSI Transfer CSW failed: %lu\n", cc);
	}
	if (csw.SIGN != CSW_SIGNATURE)
	{
		printf("USB Wrong CSW Signature: %08lx\n", csw.SIGN);
		return -1;
	}
	if (!csw.STAT)
		return 0;
	if (csw.STAT == 1)
	{
		printf("USB CSW Command Failed\n");
		return -1;
	}
	printf("USB BOT Reset\n");
	return -2;
}
void xhci_usb_msc_bot(XHCI_USB_DEVICE *device)
{
	if (!device->interface)
		return;
	STANDARD_USB_ENDPOINT *iepdesc = usb_search_endpoint(device->configuration, USB_XFER_TYPE_BULK, USB_DIR_IN);
	STANDARD_USB_ENDPOINT *oepdesc = usb_search_endpoint(device->configuration, USB_XFER_TYPE_BULK, USB_DIR_OUT);
	printf("USB Endpoint: addr=%02x, attr=%02x, max packet size=%04x, interval=%u\n", iepdesc->ADDR, iepdesc->ATTR, iepdesc->MPSZ, iepdesc->ITVL);
	printf("USB Endpoint: addr=%02x, attr=%02x, max packet size=%04x, interval=%u\n", oepdesc->ADDR, oepdesc->ATTR, oepdesc->MPSZ, oepdesc->ITVL);
	if (!iepdesc || !oepdesc)
	{
		printf("USB Mass Storage Endpoint not found\n");
		return;
	}
	DWORD iepid = xhci_endpoint_id(iepdesc);
	DWORD oepid = xhci_endpoint_id(oepdesc);
	DWORD cc;
	if ((cc = xhci_usb_configure_xfer_endpoint(device, iepdesc)))
	{
		printf("USB Mass Storage Endpoint0 fail: %lu\n", cc);
		return;
	}
	if ((cc = xhci_usb_configure_xfer_endpoint(device, oepdesc)))
	{
		printf("USB Mass Storage Endpoint1 fail: %lu\n", cc);
		return;
	}
	printf("USB Mass Storage Transfer %lu: %p\n", iepid, device->transfer[iepid]);
	printf("USB Mass Storage Transfer %lu: %p\n", oepid, device->transfer[oepid]);

	BYTE maxLun = -1;
	USB_DEVICE_SETUP_DATA requ;
	requ.RECP = USB_RECIP_INTERFACE;
	requ.RTYP = USB_RTYPE_CLASS;
	requ.DIRE = USB_RDIR_IN;
	requ.REQU = USB_REQ_GET_MAX_LUN;
	requ.VALU = 0;
	requ.INDX = device->interface->IFCN;
	requ.LENG = 1;
	if ((cc = xhci_control_transfer(device, &requ, &maxLun, 1)) != XHCI_CODE_SUCCESS)
	{
		printf("USB Mass Storage GET_MAX_LUN fail: %lu\n", cc);
		return;
	}
	printf("USB Mass Storage Max Lun: %u\n", maxLun);
	printf("USB Mass Storage Test Ready\n");
	BYTE cmd[6] = {0, 0, 0, 0, 0, 0};
	if (scsi_command(device, iepid, oepid, 1, 0, cmd, 6, 0, 0))
	{
		printf("USB Mass Storage Not Ready: %lu", cc);
		return;
	}
	printf("USB Mass Storage Ready!\n");

	SCSI_COMMAND_INQUIRY inquiry;
	__memset(&inquiry, 0, sizeof(SCSI_COMMAND_INQUIRY));
	inquiry.CODE = 0x12;
	inquiry.ALEN = 36;

	SCSI_DATA_INQUIRY data;
	__memset(cmd, 0, 6);
	__memset(&data, 0, 36);
	cmd[0] = 0x12;
	cmd[4] = 36;
	if (scsi_command(device, iepid, oepid, 1, 0, &inquiry, sizeof(inquiry), &data, 36))
	{
		printf("USB Mass Storage Inquiry Failed: %lu\n", cc);
		return;
	}
	char buf1[9];
	char buf2[17];
	char buf3[5];
	__memset(buf1, 0, sizeof(buf1));
	__memset(buf2, 0, sizeof(buf2));
	__memset(buf3, 0, sizeof(buf3));
	__memcpy(buf1, data.VEND, 8);
	__memcpy(buf2, data.PROD, 16);
	__memcpy(buf3, data.REVI, 4);
	printf("USB Massage Storage INQUIRY: %s %s %s\n", buf1, buf2, buf3);

	SCSI_READ_CAPACITY16_COMMAND rcapa;
	__memset(&rcapa, 0, sizeof(SCSI_READ_CAPACITY16_COMMAND));
	rcapa.CODE = CDB_CMD_SERVICE_ACTION_IN;
	rcapa.SACT = CDB_CMD_SAI_READ_CAPACITY_16;
	rcapa.ALEN = sizeof(SCSI_READ_CAPACITY16_DATA);
	SCSI_READ_CAPACITY16_DATA capa;
	__memset(&capa, 0, sizeof(SCSI_READ_CAPACITY16_DATA));
	if (scsi_command(device, iepid, oepid, 1, 0, &rcapa, sizeof(rcapa), &capa, sizeof(capa)))
	{
		printf("USB Mass Storage Read Capacity failed: %lu\n", cc);
		return;
	}
	printf("USB Massage Storage Capacity: LBA=%llu, sector=%lu\n", scsi_reverse8(capa.LBAX), scsi_reverse4(capa.LBLX));

	QWORD pc = 1;
	QWORD bufPhy = alloc_physical_memory(&pc, 0);
	BYTE *buf = (BYTE *) core_mapping(bufPhy);
	SCSI_READ16_COMMAND read16;
	__memset(&read16, 0, sizeof(SCSI_READ16_COMMAND));
	read16.CODE = CDB_CMD_READ_16;
	read16.LBAX = 0;
	read16.TLEN = scsi_reverse4(1);
	if (scsi_command(device, iepid, oepid, 1, 0, &read16, sizeof(read16), buf, 512))
	{
		printf("USB Mass Storage LBA failed: %lu\n", cc);
		return;
	}
	printf("%s\n", buf);
	free_physical_memory(bufPhy, 1);
}
void xhci_usb_msc_setup(XHCI_USB_DEVICE *device)
{
	printf("USB Mass Storage Device @ %p\n", device);
	STANDARD_USB_INTERFACE *iface = device->interface;
	if (!iface)
		return;
	printf("USB Interface: num=%u, setting=%u, endpoint=%u, class=%02x, subclass=%02x, proto=%u, iface=%u\n",
		iface->IFCN, iface->SETT, iface->ENDP, iface->CCOD, iface->SCOD, iface->POTO, iface->IFAC);
	if (iface->SCOD == USB_MSC_SUBCLASS_SCSI)
	{
		if (iface->POTO == USB_MSC_PROTOCOL_BOT)
			xhci_usb_msc_bot(device);
	}
}