#include <driver/scsi/scsi.h>

void swap1(BYTE *a, BYTE *b)
{
	BYTE c = *a;
	*a = *b;
	*b = c;
}
WORD scsi_reverse2(WORD x)
{
	return (WORD) ((x << 8) | (x >> 8));
}
DWORD scsi_reverse4(DWORD x)
{
	DWORD y = x;
	BYTE *data = (BYTE*) &y;
	swap1(data, data + 3);
	swap1(data + 1, data + 2);
	return y;
}
QWORD scsi_reverse8(QWORD x)
{
	QWORD y = x;
	DWORD *data = (DWORD*) &y;
	DWORD c = scsi_reverse4(data[0]);
	data[0] = scsi_reverse4(data[1]);
	data[1] = c;
	return y;
}
void scsi_reverse(BYTE *data, DWORD len)
{
	for (DWORD i = 0; i < (len >> 1); i++)
		swap1(data + i, data + (len - i - 1));
}