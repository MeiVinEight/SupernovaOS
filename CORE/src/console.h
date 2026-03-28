#pragma once
#include <types.h>

void setup_basic_console();
void setup_console();
void draw_char(char ch, DWORD color, DWORD x, DWORD y);
void outchar(char ch);
void simple_output(const void *);
void simple_output_number(QWORD x);
void simple_output_address(QWORD val, BYTE cnt);