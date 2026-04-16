#include <user/user.h>
#include <intrinsic.h>
#include <stdio.h>

void user_main()
{
	cprintf(0x0A, "SupernovaOS User space\n");
	*((DWORD *) 1) = 0;
	while (1) _mm_pause();
}
