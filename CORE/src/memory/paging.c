#include <intrinsic.h>

void setup_paging()
{
	// Setup Paging
	QWORD *paging = (QWORD *) 0x1000;
	paging[0x000] = 0x2003;
	paging[0x100] = 0x2003;
	paging = (QWORD *) 0x2000;
	paging[0] = 0x3003;
	paging[1] = 0x4003;
	paging[2] = 0x5003;
	paging[3] = 0x6003;
	QWORD memAddr = 0;
	for (QWORD i = 0; i < 4; i++)
	{
		paging = (QWORD *) (0x3000 + (i * 0x1000));
		for (QWORD j = 0; j < 512; j++)
		{
			paging[j] = memAddr | 0x183;
			memAddr += 0x00200000;
		}
	}
	__writecr3(0x1000);
}