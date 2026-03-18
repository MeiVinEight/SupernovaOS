#pragma once

#include <acpi/acpi.h>

#define CPUID_FEAT_EDX_APIC (1 << 9)

#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

#define APIC_SOFTWARE_ENABLE 0x100
#define APIC_SPURIOUS_VECTOR 0xFF

#define APIC_APICID  0x02
#define APIC_APICVER 0x03
#define APIC_TPR     0x08
#define APIC_APR     0x09
#define APIC_PPR     0x0A
#define APIC_EOI     0x0B
#define APIC_RRD     0x0C
#define APIC_LDR     0x0D
#define APIC_DFR     0x0E
#define APIC_SIV     0x0F
#define APIC_ISR     0x10
#define APIC_TMR     0x18
#define APIC_IRR     0x20
#define APIC_ESR     0x28
#define APIC_CMCI    0x2F
#define APIC_ICRL    0x30
#define APIC_ICRH    0x31
#define APIC_LVT0    0x32
#define APIC_LVT1    0x33
#define APIC_LVT2    0x34
#define APIC_LVT3    0x35
#define APIC_LVT4    0x36
#define APIC_LVTE    0x37
#define APIC_TICR    0x38
#define APIC_TCCR    0x39
#define APIC_TDCR    0x3E

#define APIC_ICR_DELIVERY_INIT       0x00000500
#define APIC_ICR_DELIVERY_STARTUP    0x00000600
#define APIC_ICR_DELIVERY_STATUS     0x00001000
#define APIC_ICR_LEVEL_ASSERT        0x00004000
#define APIC_ICR_TRIGGER_MODE_LEVEL  0x00008000
#define APIC_ICR_SHORTHAND_EXCLUDING 0x000C0000

#define APIC_LVT_NMI (1 << 10)
#define APIC_LVT_CLR (1 << 16)

#define APIC_TIMER_IRQ 0
#define APIC_TIMER_MODE_PERIODIC (1 << 17)
#define APIC_TIMER_DCR_1   0xB
#define APIC_TIMER_DCR_2   0x0
#define APIC_TIMER_DCR_4   0x1
#define APIC_TIMER_DCR_8   0x2
#define APIC_TIMER_DCR_16  0x3
#define APIC_TIMER_DCR_32  0x8
#define APIC_TIMER_DCR_64  0x9
#define APIC_TIMER_DCR_128 0xA

typedef struct _ACPI_MADT
{
	ACPI_SDT_HEADER HEAD;
	DWORD LAAA; // Local APIC Address
	DWORD FLAG; // 1 = Dual 8259 Legacy PICs Instlled
	BYTE  DATA[];
} ACPI_MADT;
typedef struct _APIC_MADT_LAPIC
{
	BYTE  TYP; // Type, 0 is Local APIC
	BYTE  SZE; // Always 8
	BYTE  PID; // APIC Processor ID
	BYTE  AID; // APIC ID
	DWORD FLG; // Flags
} APIC_MADT_LAPIC;

extern DWORD USEAPIC;
extern volatile DWORD (*volatile APIC_REGISTERS)[4];

int check_apic();
void setup_apic();
DWORD apic_current_id();
void setup_apic_timer(DWORD rate);
void eoi_apic(BYTE id);
void apic_ipi(BYTE apicId, BYTE intr);
void apic_startup_ap(BYTE apicid, void (*apEntry)(void));
void setup_madt(ACPI_MADT *madt);
void apic_setup_multiprocessor();