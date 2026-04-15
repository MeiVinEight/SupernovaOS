#pragma once
#include <types.h>

typedef struct _SIMPLE_TEXT_MODE
{
	DWORD POS;
	DWORD COLOR;
} SIMPLE_TEXT_MODE;

extern SIMPLE_TEXT_MODE SIMPLE_TEXT;

void setup_basic_console();
void setup_console();
void draw_char(char ch, DWORD color, DWORD x, DWORD y);
void outchar(char ch);