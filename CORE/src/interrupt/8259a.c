#include <interrupt/8259a.h>
#include <intrinsic.h>
#include <console.h>

void setup_8259A()
{
	disable_8259A();
	printf("8259A PIC OFFLINE\n");
}
void disable_8259A()
{
	__outbyte(PIC2_DATA, ICW1_DISABLE);
	__outbyte(PIC1_DATA, ICW1_DISABLE);
}
void eoi_8259A(BYTE id)
{
	if (id < 0x30)
	{
		if (id > 0x27)
		{
			__outbyte(PIC2_COMMAND, ICW0_EOI);
		}
		__outbyte(PIC1_COMMAND, ICW0_EOI);
	}
}