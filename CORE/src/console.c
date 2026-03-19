#include <console.h>
#include <intrinsic.h>
#include <core.h>
#include <memory/virtmem.h>

COREAPI char draw_char_lines[] = {
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
COREAPI char scroll_text_line[] = {
	// LOOP0:
	0x48, 0x85, 0xC9,             // TEST RCX, RCX
	0x74, 0x1F,                   // JZ LOOP1

	0xF3, 0x0F, 0x6F, 0x02,       // MOVDQU XMM0, XMMWORD PTR [RDX]
	0xF3, 0x41, 0x0F, 0x7F, 0x00, // MOVDQU XMMWORD PTR [R8], XMM0
	0xF3, 0x41, 0x0F, 0x7F, 0x01, // MOVDQU XMMWORD PTR [R9], XMM0
	0x48, 0x83, 0xC2, 0x10,       // ADD RDX, 10H
	0x49, 0x83, 0xC0, 0x10,       // ADD R8, 10H
	0x49, 0x83, 0xC1, 0x10,       // ADD R9, 10H
	0x48, 0xFF, 0xC9,             // DEC RCX
	0xEB, 0xDC,                   // JMP LOOP0

	// LOOP1:
	0xC3,                   // RET
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
COREAPI volatile QWORD FRAME_BUFFER = 0;
COREAPI volatile QWORD FLUSH_BUFFER = 0;

void setup_basic_console()
{
	QWORD buf[3] = { 0, 0, 0 };
	QWORD *avxBuf = buf;
	if (((QWORD) avxBuf) & 0xF)
		avxBuf++;

	FRAME_BUFFER = core_mapping(SYSTEM_TABLE->FBB);
	__memset128((void *) FRAME_BUFFER, avxBuf, (SYSTEM_TABLE->PPL * SYSTEM_TABLE->VRES * 4) / 16);

}
void setup_console()
{
	QWORD memSpace = SYSTEM_TABLE->VRES * SYSTEM_TABLE->PPL * 4;
	memSpace += 0x1FFFFF;
	memSpace >>= 21;
	QWORD virtualAddr = 0xFFFF808000000000ULL;
	QWORD flushAddr = virtualAddr;
	QWORD pageCount = 512;
	while (memSpace--)
	{
		QWORD phyAddr = alloc_physical_memory(&pageCount, 21, 1);
		if (!phyAddr)
		{
			simple_output("INSUFFICIENT MEMORY FOR VIDEO");
			while (1) __halt();
		}
		virtual_mapping(phyAddr, virtualAddr, 1, PAGE_2M);
		virtualAddr += (1 << 21);
	}
	FLUSH_BUFFER = flushAddr;
	QWORD buf[3] = { 0, 0, 0 };
	QWORD *avxBuf = buf;
	if (((QWORD) avxBuf) & 0xF)
		avxBuf++;
	__memset128((void *) FLUSH_BUFFER, avxBuf, (SYSTEM_TABLE->PPL * SYSTEM_TABLE->VRES * 4) / 16);
}
void draw_char(char ch, DWORD color, DWORD x, DWORD y)
{
	volatile BYTE(*fontMap)[16] = SYSTEM_TABLE->FONT;
	volatile BYTE *font = fontMap[ch];
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
	((void (*)(QWORD, void *, QWORD)) draw_char_lines)(FRAME_BUFFER + (pixelPos * 4), CHAR_BUFFER, SYSTEM_TABLE->PPL * 4);
	if (!FLUSH_BUFFER)
		return;
	((void (*)(QWORD, void *, QWORD)) draw_char_lines)(FLUSH_BUFFER + (pixelPos * 4), CHAR_BUFFER, SYSTEM_TABLE->PPL * 4);
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
		if (!FLUSH_BUFFER)
		{
			QWORD copySize = ((SYSTEM_TABLE->VRES - 16) * SYSTEM_TABLE->PPL) >> 2;
			QWORD *src = (QWORD *) (FRAME_BUFFER + SYSTEM_TABLE->PPL * 16 * 4);
			QWORD *dst = (QWORD *) (FRAME_BUFFER);
			__memcpy128(dst, src, copySize);
			SIMPLE_TEXT.POS -= charPreLine;
			QWORD buf[2] = { 0, 0 };
			__memset128((void *) (FRAME_BUFFER + ((maxLines - 1) * 64 * SYSTEM_TABLE->PPL)), buf, SYSTEM_TABLE->PPL * 4);
			return;
		}
		QWORD copySize = ((SYSTEM_TABLE->VRES - 16) * SYSTEM_TABLE->PPL) >> 2;
		QWORD src = (FLUSH_BUFFER + SYSTEM_TABLE->PPL * 16 * 4);
		//while (copySize--)
		//	*(dst++) = *(src++);
		//__memcpy128(dst, src, copySize);
		((void (*)(QWORD, QWORD, QWORD, QWORD)) scroll_text_line)(copySize, src, FRAME_BUFFER, FLUSH_BUFFER);
		//dst = (QWORD *) FLUSH_BUFFER;
		//__memcpy128(dst, src, copySize);
		QWORD buf[2] = { 0, 0 };
		__memset128((void *) (FRAME_BUFFER + ((maxLines - 1) * 64 * SYSTEM_TABLE->PPL)), buf, SYSTEM_TABLE->PPL * 4);
		__memset128((void *) (FLUSH_BUFFER + ((maxLines - 1) * 64 * SYSTEM_TABLE->PPL)), buf, SYSTEM_TABLE->PPL * 4);
		SIMPLE_TEXT.POS -= charPreLine;
	}
}
void simple_output(const void *buf)
{
	const char *str = buf;
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
			buf[--idx] = (char) ('0' + (x % 10));
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