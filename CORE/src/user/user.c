#include <user/user.h>
#include <intrinsic.h>

void user_main()
{
	__syscall(0);
	while (1) _mm_pause();
}
