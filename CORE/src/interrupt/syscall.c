#include <interrupt/syscall.h>
#include <interrupt/interrupt.h>
#include <core.h>
#include <mm/vmm.h>
#include <arch/tss.h>
#include <intrinsic.h>
#include <std/string.h>

void interrupt_system_call(INTERRUPT_STACK *stack)
{
	if (!(stack->CS & 3))
	{
		stack->RIP = *((QWORD *) stack->RSP);
		panic(stack);
	}

	if (!stack->RCX)
		return;

	QWORD type = *((QWORD *) stack->RCX);
	if (type == SYSCALL_TYPE_PRINTF)
	{
		SYSCALL_PRINTF *arg = (SYSCALL_PRINTF *) stack->RCX;
		stack->RAX = vprintf(arg->ATTR, arg->FMRT, arg->VARG);
	}
}
void setup_system_call()
{
	BYTE vec = SYSCALL_INT_VECTOR;
	interrupt_set_intx(vec);
	IDT[vec].PRVL = 3;
	QWORD pc = 1;
	QWORD rsp0 = core_mapping(alloc_physical_memory(&pc, 0, 0));
	TASK_STATE_SEGMENT *tss = (TASK_STATE_SEGMENT *) SYSTEM_TABLE->TSS;
	__memset(tss, 0, sizeof(TASK_STATE_SEGMENT));
	tss->RSPX[0] = rsp0;
	register_interrupt(vec, interrupt_system_call);
}