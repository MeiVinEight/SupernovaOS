#include <console.h>


__declspec(allocate(".text")) const char draw_char_lines[] = {
	0xB8, 0x10, 0x00, 0x00, 0x00, // MOV EAX, 10H

	// LOOP0:
	0x85, 0xC0,                   // TEST EAX, EAX
	0x74, 0x1D,                   // JZ   LOOP1

	0xF3, 0x0F, 0x6F, 0x02,       // MOVDQU XMM0, XMMWORD PTR [RDX]
	0xF3, 0x0F, 0x7F, 0x01,       // MOVDQU XMMWORD PTR [RCX], XMM0
	0xF3, 0x0F, 0x6F, 0x42, 0x10, // MOVDQU XMM0, XMMWORD PTR [RDX + 10H]
	0xF3, 0x0F, 0x7F, 0x41, 0x10, // MOVDQU XMMWORD PTR [RCX + 10H], XMM0
	0x49, 0x03, 0xC8,             // ADD RCX, R8
	0x48, 0x83, 0xC2, 0x20,       // ADD RDX, 20H
	0xFF, 0xC8,                   // DEC RAX
	0xEB, 0xDF,                   // JMP LOOP0

	// LOOP1:
	0xC3                          // RET
};

COREAPI DWORD CHAR_BUFFER[128];
COREAPI DWORD COLOR_PLAETTE[] =
{
	0x000000,
	0x0000AA,
	0x00AA00,
	0x00AAAA,
	0xAA0000,
	0xAA00AA,
	0xFFAA00,
	0xAAAAAA,
	0x555555,
	0x5555FF,
	0x55FF55,
	0x55FFFF,
	0xFF5555,
	0xFF55FF,
	0xFFFF55,
	0xFFFFFF
};
COREAPI SIMPLE_TEXT_MODE SIMPLE_TEXT = {
	.POS = 0,
	.COLOR = 0x0F
};

void draw_char(char ch, DWORD color, DWORD x, DWORD y)
{
	BYTE(*fontMap)[16] = (BYTE(*)[16]) 0x7000;
	BYTE *font = fontMap[ch];
	DWORD colorMap[2] = {COLOR_PLAETTE[(color >> 4) & 0xF], COLOR_PLAETTE[(color >> 0) & 0xF]};
	for (int i = 0; i < 16; i++)
	{
		BYTE fontBitLine = font[i];
		for (int j = 8; j--;)
		{
			CHAR_BUFFER[i * 8 + j] = colorMap[fontBitLine & 1];
			fontBitLine >>= 1;
		}
	}
	QWORD pixelPos = y * 16 * SYSTEM_TABLE->PPL;
	pixelPos += x * 8;
	((void (*)(QWORD, void *, QWORD)) draw_char_lines)(SYSTEM_TABLE->FBB + (pixelPos * 4), CHAR_BUFFER, SYSTEM_TABLE->PPL * 4);
}
void outchar(char ch)
{
	DWORD charPreLine = SYSTEM_TABLE->HRES / 8;
	if (ch == '\r')
	{
		SIMPLE_TEXT.POS -= SIMPLE_TEXT.POS % charPreLine;
	}
	else if (ch == '\n')
	{
		SIMPLE_TEXT.POS += charPreLine - (SIMPLE_TEXT.POS % charPreLine);
	}
	else
	{
		draw_char(ch, SIMPLE_TEXT.COLOR, SIMPLE_TEXT.POS % charPreLine, SIMPLE_TEXT.POS / charPreLine);
		SIMPLE_TEXT.POS++;
	}
	DWORD maxLines = SYSTEM_TABLE->VRES / 16;
	DWORD lines = SIMPLE_TEXT.POS / charPreLine;
	if (lines >= maxLines)
	{
		QWORD copySize = (SYSTEM_TABLE->VRES - 16) * SYSTEM_TABLE->PPL / 2;
		QWORD *dst = (QWORD *) (SYSTEM_TABLE->FBB);
		QWORD *src = (QWORD *) (SYSTEM_TABLE->FBB + SYSTEM_TABLE->PPL * 16 * 4);
		while (copySize--)
			*(dst++) = *(src++);
		QWORD buf[2] = { 0, 0 };
		__memset128((void *) (SYSTEM_TABLE->FBB + ((maxLines - 1) * 64 * SYSTEM_TABLE->PPL)), buf, SYSTEM_TABLE->PPL * 4);
		SIMPLE_TEXT.POS -= charPreLine;
	}
}
void simple_output(const void *buf)
{
	const char *str = (const char *) buf;
	while (*str)
		outchar(*str++);
}
void simple_output_number(QWORD x)
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
	simple_output(num);
}
void simple_output_address(QWORD val, BYTE cnt)
{
	char hex[] = "0123456789ABCDEF";
	char buf[17];
	buf[cnt] = 0;
	for (int i = cnt; i--;)
	{
		buf[i] = hex[val & 0xF];
		val >>= 4;
	}
	simple_output(buf);
}