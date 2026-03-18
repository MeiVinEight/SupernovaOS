#pragma once
#include <types.h>

typedef struct _SIMPLE_TEXT_MODE
{
	volatile DWORD POS;
	volatile DWORD COLOR;
} SIMPLE_TEXT_MODE;

void draw_char(char ch, DWORD color, DWORD x, DWORD y);
void outchar(char ch);
void simple_output(const void *);
void simple_output_number(QWORD x);
void simple_output_address(QWORD val, BYTE cnt);