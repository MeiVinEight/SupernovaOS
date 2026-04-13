#include <core.h>
#include <intrinsic.h>

COREAPI DWORD _fltused = 1;

QWORD core_mapping(QWORD addr)
{
	return addr | SYSTEM_ADDRESS;
}