#ifndef __STD_STRING_H__
#define __STD_STRING_H__

#include <stdint.h>

int vsnprintf(char* buf, size_t n, const char* fmt, va_list ap);
int vsprintf(char* buf, const char* fmt, va_list ap);
int __cdecl sprintf(char* buf, const char* fmt, ...);
int __cdecl snprintf(char* buf, size_t n, const char* fmt, ...);

#endif