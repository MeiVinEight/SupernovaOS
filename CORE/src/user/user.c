#include <user/user.h>
#include <intrinsic.h>
#include <stdio.h>

void user_main()
{
	cprintf(0x0A, "SupernovaOS User space\n");
	while (1) _mm_pause();
}
