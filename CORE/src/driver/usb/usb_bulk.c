#include <driver/usb/usb_bulk.h>
#include <driver/disk/disk.h>
#include <driver/xhci/xhci_device.h>
#include <driver/xhci/xhci_context.h>
#include <driver/scsi/scsi.h>
#include <driver/scsi/scsi_cmd.h>
#include <mm/vmm.h>
#include <intrinsic.h>
#include <core.h>
#include <string.h>

typedef struct _USB_BULK_STORAGE_DEVICE
{
	STANDARD_STORAGE_DEVICE XSSD;
	XHCI_USB_DEVICE        *XUSB;
	SCSI_DATA_INQUIRY       IQRY;
	BYTE                    IEPI;
	BYTE                    OEPI;
} USB_BULK_STORAGE_DEVICE;

DWORD scsi_command(XHCI_USB_DEVICE *device, DWORD iepid, DWORD oepid, DWORD isIn, DWORD lun, const void *cmd, DWORD cmdLen, void *data, DWORD dataLen)
{
	if ((QWORD) data & 0xFFF)
		return 1; // Un-aligned address

	BYTE *buf = (BYTE *) core_mapping(device->persistent);
	USB_MASS_STORAGE_BULK_CBW *cbw = (USB_MASS_STORAGE_BULK_CBW *) buf;
	__memset(cbw, 0, sizeof(USB_MASS_STORAGE_BULK_CBW));
	cbw->SIGN = CBW_SIGNATURE;
	cbw->TAGX = 1;
	cbw->DTRL = dataLen;
	cbw->FLAG = (dataLen && isIn) ? USB_DIR_IN : USB_DIR_OUT;
	cbw->CLUN = lun;
	cbw->CLEN = cmdLen;
	__memcpy(cbw->CMMD, cmd, cmdLen);
	DWORD cc;
	if ((cc = xhci_transfer(device, oepid, 1, 0, (void *) core_mapping(device->persistent), 31)) != XHCI_CODE_SUCCESS)
	{
		return 0x80 + cc;
	}
	if (dataLen)
	{
		if ((cc = xhci_transfer(device, (isIn ? iepid : oepid), 1, 0, data, dataLen)) != XHCI_CODE_SUCCESS)
		{
			return 0x80 + cc;
		}
	}
	USB_MASS_STORAGE_BULK_CSW *csw = (USB_MASS_STORAGE_BULK_CSW *) buf;
	csw->SIGN = 0;
	if ((cc = xhci_transfer(device, iepid, 1, 0, csw, 13)) != XHCI_CODE_SUCCESS)
	{
		return 0x80 + cc;
	}
	if (csw->SIGN != CSW_SIGNATURE)
	{
		return 2;
	}
	if (!csw->STAT)
		return 0;
	if (csw->STAT == 1)
	{
		return 3;
	}
	return 4;
}
QWORD usb_bulk_read(STANDARD_STORAGE_DEVICE *dev, void *buf, QWORD lba, DWORD sec)
{
	if ((QWORD) buf & 0xFFF)
	{
		void *bufx = storage_dma_buffer(dev);
		QWORD cc = 0;
		while (sec)
		{
			DWORD read = sec;
			if (sec > 8)
				read = 8;
			if ((cc = usb_bulk_read(dev, bufx, lba, read)))
				break;
			__memcpy(buf, bufx, (read << 9));
			buf = ((BYTE *) buf) + (read << 9);
			lba += read;
			sec -= read;
		}
		return cc;
	}

	USB_BULK_STORAGE_DEVICE *device = (USB_BULK_STORAGE_DEVICE *) dev;

	BYTE *addr = buf;
	while (sec)
	{
		DWORD rsec = sec;
		if (rsec > 8)
			rsec = 8;
		SCSI_READ16_COMMAND read16;
		__memset(&read16, 0, sizeof(SCSI_READ16_COMMAND));
		read16.CODE = CDB_CMD_READ_16;
		read16.LBAX = scsi_reverse8(lba);
		read16.TLEN = scsi_reverse4(rsec);
		DWORD cc;
		(void) *((volatile BYTE *) addr); // Make sure physical page allocated
		if ((cc = scsi_command(device->XUSB, device->IEPI, device->OEPI, 1, 0, &read16, sizeof(read16), addr, ((QWORD) rsec) << 9)))
			return cc;
		lba += rsec;
		sec -= rsec;
		addr += rsec << 9;
	}

	return 0;
}
DWORD xhci_usb_msc_bot(XHCI_USB_DEVICE *device)
{
	if (!device->interface)
		return 1;
	STANDARD_USB_ENDPOINT *iepdesc = usb_search_endpoint(device->configuration, USB_XFER_TYPE_BULK, USB_DIR_IN);
	STANDARD_USB_ENDPOINT *oepdesc = usb_search_endpoint(device->configuration, USB_XFER_TYPE_BULK, USB_DIR_OUT);
	if (!iepdesc || !oepdesc)
		return 2;
	DWORD iepid = xhci_endpoint_id(iepdesc);
	DWORD oepid = xhci_endpoint_id(oepdesc);
	DWORD cc;
	if ((cc = xhci_usb_configure_xfer_endpoint(device, iepdesc)))
		return 3;
	if ((cc = xhci_usb_configure_xfer_endpoint(device, oepdesc)))
		return 4;

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
		return 5;
	BYTE cmd[6] = {0, 0, 0, 0, 0, 0};
	if ((cc = scsi_command(device, iepid, oepid, 1, 0, cmd, 6, 0, 0)))
		return 6;

	SCSI_COMMAND_INQUIRY inquiry;
	__memset(&inquiry, 0, sizeof(SCSI_COMMAND_INQUIRY));
	inquiry.CODE = 0x12;
	inquiry.ALEN = sizeof(SCSI_DATA_INQUIRY);

	QWORD phyAddr = alloc_physical_memory(1, 0);
	BYTE *buf = (BYTE *) core_mapping(phyAddr);

	SCSI_DATA_INQUIRY *data = (SCSI_DATA_INQUIRY *) buf;
	__memset(data, 0, sizeof(SCSI_DATA_INQUIRY) + 1);
	if ((cc = scsi_command(device, iepid, oepid, 1, 0, &inquiry, sizeof(inquiry), data, sizeof(SCSI_DATA_INQUIRY))))
		return 7;

	USB_BULK_STORAGE_DEVICE *ssd = heap_alloc(sizeof(USB_BULK_STORAGE_DEVICE));
	__memset(ssd, 0, sizeof(USB_BULK_STORAGE_DEVICE));
	ssd->IQRY = *data;
	__memcpy(ssd->XSSD.TEXT, ssd->IQRY.VEND, 8);
	strtrim(ssd->XSSD.TEXT);
	__memcpy(ssd->XSSD.TEXT + strlen(ssd->XSSD.TEXT), ssd->IQRY.PROD, 16);
	strtrim(ssd->XSSD.TEXT);
	ssd->XSSD.MODN = ssd->XSSD.TEXT;

	SCSI_READ_CAPACITY16_COMMAND rcapa;
	__memset(&rcapa, 0, sizeof(SCSI_READ_CAPACITY16_COMMAND));
	rcapa.CODE = CDB_CMD_SERVICE_ACTION_IN;
	rcapa.SACT = CDB_CMD_SAI_READ_CAPACITY_16;
	rcapa.ALEN = sizeof(SCSI_READ_CAPACITY16_DATA);
	SCSI_READ_CAPACITY16_DATA *capa = (SCSI_READ_CAPACITY16_DATA *) buf;;
	__memset(capa, 0, sizeof(SCSI_READ_CAPACITY16_DATA));
	if ((cc = scsi_command(device, iepid, oepid, 1, 0, &rcapa, sizeof(rcapa), capa, sizeof(SCSI_READ_CAPACITY16_DATA))))
	{
		heap_free(ssd);
		return 8;
	}

	ssd->XSSD.READ = usb_bulk_read;
	ssd->XSSD.CAPA = scsi_reverse8(capa->LBAX);
	ssd->XUSB = device;
	ssd->IEPI = iepid;
	ssd->OEPI = oepid;
	storage_insert(&ssd->XSSD);
	free_physical_memory(phyAddr, 1);
	return 0;
}