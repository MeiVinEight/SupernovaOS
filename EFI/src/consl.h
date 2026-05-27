#pragma once

#include <intrinsic.h>
#include <uefi.h>

typedef struct _SIMPLE_TEXT_MODE
{
	DWORD  POSS;
	DWORD  COLR;
	BYTE (*FONT)[16];
	DWORD  HRES;
	DWORD  VRES;
	DWORD  PPLN;
} SIMPLE_TEXT_MODE;

void setup_console(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *mode, BYTE *font);
void draw_char(char ch, DWORD color, DWORD x, DWORD y);
void outchar(char ch);