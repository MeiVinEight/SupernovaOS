#pragma once

#include <types.h>

typedef struct _REENTRANT_LOCK
{
	WORD CORE;
	WORD CONT;
} REENTRANT_LOCK;

void async_lock(REENTRANT_LOCK *lock);
void async_unlock(REENTRANT_LOCK *lock);