#pragma once

#include <types.h>

typedef struct _PROCESS_CONTROL_BLOCK PROCESS_CONTROL_BLOCK;
struct _PROCESS_CONTROL_BLOCK
{
	PROCESS_CONTROL_BLOCK *NEXT;
	void                  *VMMA;
	WORD                   PCID;
};

PROCESS_CONTROL_BLOCK *create_process();
PROCESS_CONTROL_BLOCK *current_process();