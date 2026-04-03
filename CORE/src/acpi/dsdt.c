#include <types.h>
#include <acpi/dsdt.h>

QWORD dsdt_parse_nameseg(BYTE *dsdt, BYTE *buf)
{
	if (!dsdt)
		return 0;
	if (buf)
		*((DWORD *) buf) = *((DWORD *) dsdt);
	return 4;
}
QWORD dsdt_parse_multinamepath(BYTE *dsdt, BYTE *buf)
{
	if (!dsdt)
		return 0;
	if (*dsdt != '/')
		return 0;
	dsdt++;
	BYTE segCnt = *dsdt;
	dsdt++;
	QWORD ret = 2 + ((QWORD) segCnt << 2);
	if (buf)
	{
		while (segCnt--)
		{
			QWORD rl = dsdt_parse_nameseg(dsdt, buf);
			dsdt += rl;
			buf += rl;
		}
	}
	return ret;
}
QWORD dsdt_parse_dualnamepath(BYTE *dsdt, BYTE *buf)
{
	if (!dsdt)
		return 0;
	if (*dsdt != '.')
		return 0;
	if (buf)
	{
		dsdt_parse_nameseg(dsdt, buf);
		dsdt += 4;
		buf += 4;
		dsdt_parse_nameseg(dsdt, buf);
	}
	return 9;
}
QWORD dsdt_parse_prefixpath(BYTE *dsdt, BYTE *buf)
{
	if (!dsdt)
		return 0;
	if (*dsdt != '^')
		return 0;
	if (buf)
		*buf++ = '^';
	return 1 + dsdt_parse_prefixpath(dsdt + 1, buf);;
}
QWORD dsdt_parse_namestring(BYTE *dsdt, BYTE *buf)
{
	if (!dsdt)
		return 0;
	if (*dsdt == '\\')
		*buf++ = '\\';
	return 1;
}