#include <async/async.h>
#include <arch/processor.h>
#include <intrinsic.h>

void async_lock(REENTRANT_LOCK *lock0)
{
	volatile REENTRANT_LOCK *lock = lock0;
	short core = (short) (cpu_local_apic_id() + 1);
	while (core != lock->CORE)
		_InterlockedCompareExchange16((short *) &lock->CORE, core, 0);
	lock->CONT++;
}
void async_unlock(REENTRANT_LOCK *lock0)
{
	volatile REENTRANT_LOCK *lock = lock0;
	lock->CONT--;
	if (!lock->CONT)
		lock->CORE = 0;
}