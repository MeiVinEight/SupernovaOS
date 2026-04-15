#pragma once

#include <types.h>
#include <interrupt/interrupt.h>

#define SMP_INT_VECTOR 0x2F

void int_smp(INTERRUPT_STACK *stack);