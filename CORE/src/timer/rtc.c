#include <timer/rtc.h>
#include <intrinsic.h>

int rtc_update_in_progress()
{
	__outbyte(0x70, 0x0A);
	return (__inbyte(0x71) & 0x80);
}
BYTE rtc_get_second()
{
	while (rtc_update_in_progress()) __nop();
	__outbyte(0x70, 0x00);
	return __inbyte(0x71);
}