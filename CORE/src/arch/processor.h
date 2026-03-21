#pragma once

#include <types.h>

void setup_processor();
BYTE cpu_local_apic_id();
void cpuid_brand(char *buf);
int cpu_supports_huge_page();