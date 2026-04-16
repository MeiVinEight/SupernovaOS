#include <core.h>
#include <intrinsic.h>
#include <stdio.h>
#include <console.h>
#include <arch/processor.h>
#include <memory/segment.h>
#include <arch/rflag.h>
#include <arch/efer.h>
#include <arch/msr.h>

COREAPI DWORD _fltused = 1;

QWORD core_mapping(QWORD addr)
{
	return addr | SYSTEM_ADDRESS;
}
void panic(INTERRUPT_STACK *stack)
{
	SIMPLE_TEXT.COLOR = 0x0C;
	printf("PANIC CPU #%u INT: #%02llX @ %04llX:%016llX\nERR=%016llX\n", cpu_local_apic_id(), stack->INT, stack->CS, stack->RIP, stack->ERROR);
	printf("RAX=%016llX RCX=%016llX RDX=%016llX RBX=%016llX\n", stack->RAX, stack->RCX, stack->RDX, stack->RBX);
	printf("RSP=%016llX RBP=%016llX RSI=%016llX RDI=%016llX\n", stack->RSP, stack->RBP, stack->RSI, stack->RDI);
	printf("R8 =%016llX R9 =%016llX R10=%016llX R11=%016llX\n", stack->R8, stack->R9, stack->R10, stack->R11);
	printf("R12=%016llX R13=%016llX R14=%016llX R15=%016llX\n", stack->R12, stack->R13, stack->R14, stack->R15);
	printf("CR0=%016llX CR2=%016llX CR3=%016llX CR4=%016llX\nCR8=%016llX\n", __readcr0(), __readcr2(), __readcr3(), __readcr4(), __readcr8());
	printf("DR0=%016llX DR1=%016llX DR2=%016llX DR3=%016llX\nDR6=%016llX DR7=%016llX\n", __readdr(0), __readdr(1), __readdr(2), __readdr(3), __readdr(6), __readdr(7));
	WORD es = __getes();
	WORD cs = stack->CS;
	WORD ss = __getss();
	if (stack->CS & 3)
		ss = stack->SS;
	WORD ds = __getds();
	WORD fs = __getfs();
	WORD gs = __getgs();
	WORD tr = __str();
	WORD lr = __sldt();
	GDTR64 gdtr;
	IDTR64 idtr;
	__sgdt(&gdtr);
	__sidt(&idtr);
	GLOBAL_SEGMENT_DESCRIPTOR *gdt = (GLOBAL_SEGMENT_DESCRIPTOR *) SYSTEM_TABLE->GDT;
	GLOBAL_SEGMENT_DESCRIPTOR *ess = gdt + (es >> 3);
	GLOBAL_SEGMENT_DESCRIPTOR *css = gdt + (cs >> 3);
	GLOBAL_SEGMENT_DESCRIPTOR *sss = gdt + (ss >> 3);
	GLOBAL_SEGMENT_DESCRIPTOR *dss = gdt + (ds >> 3);
	GLOBAL_SEGMENT_DESCRIPTOR *fss = gdt + (fs >> 3);
	GLOBAL_SEGMENT_DESCRIPTOR *gss = gdt + (gs >> 3);
	SYSTEM_SEGMENT_DESCRIPTOR *tss = (SYSTEM_SEGMENT_DESCRIPTOR *) (gdt + (tr >> 3));
	SYSTEM_SEGMENT_DESCRIPTOR *lss = (SYSTEM_SEGMENT_DESCRIPTOR *) (gdt + (lr >> 3));
	QWORD esa = (ess->RSV0 >> 16) | (ess->RSV1 << 16) | (ess->RSV3 << 24);
	QWORD csa = (css->RSV0 >> 16) | (css->RSV1 << 16) | (css->RSV3 << 24);
	QWORD ssa = (sss->RSV0 >> 16) | (sss->RSV1 << 16) | (sss->RSV3 << 24);
	QWORD dsa = (dss->RSV0 >> 16) | (dss->RSV1 << 16) | (dss->RSV3 << 24);
	QWORD fsa = (fss->RSV0 >> 16) | (fss->RSV1 << 16) | (fss->RSV3 << 24);
	QWORD gsa = (gss->RSV0 >> 16) | (gss->RSV1 << 16) | (gss->RSV3 << 24);
	QWORD tsa = (tss->ADD0) | (tss->ADD1 << 16) | (tss->ADD2 << 24) | ((QWORD) tss->ADD3 << 32);
	QWORD lsa = (lss->ADD0) | (lss->ADD1 << 16) | (lss->ADD2 << 24) | ((QWORD) lss->ADD3 << 32);
	DWORD esl = (ess->RSV0 & 0xFFFF) | ((ess->RSV2 & 0xF) << 16);
	DWORD csl = (css->RSV0 & 0xFFFF) | ((css->RSV2 & 0xF) << 16);
	DWORD ssl = (sss->RSV0 & 0xFFFF) | ((sss->RSV2 & 0xF) << 16);
	DWORD dsl = (dss->RSV0 & 0xFFFF) | ((dss->RSV2 & 0xF) << 16);
	DWORD fsl = (fss->RSV0 & 0xFFFF) | ((fss->RSV2 & 0xF) << 16);
	DWORD gsl = (gss->RSV0 & 0xFFFF) | ((gss->RSV2 & 0xF) << 16);
	DWORD tsl = (tss->LIMT) | (tss->ZRO1 << 16);
	DWORD lsl = (lss->LIMT) | (lss->ZRO1 << 16);
	char palette[] = "-G-D-L-A-P-S";
	printf("ES =%04X %016llX:%05lX TYPE=%X DPL=%u [%c%c%c%c%c%c]\n", es, esa, esl, (WORD) (SYSTEM_TABLE->GDT[es >> 3] >> 40) & 0xF, ess->DPLX,palette[ess->GRAN], palette[2 + ess->SIZE], palette[4 + ess->LONG], palette[6 + (ess->RSV2 >> 4)], palette[8 + ess->PRST], palette[10 + ess->SEGT]);
	printf("CS =%04X %016llX:%05lX TYPE=%X DPL=%u [%c%c%c%c%c%c]\n", cs, csa, csl, (WORD) (SYSTEM_TABLE->GDT[cs >> 3] >> 40) & 0xF, css->DPLX,palette[css->GRAN], palette[2 + css->SIZE], palette[4 + css->LONG], palette[6 + (css->RSV2 >> 4)], palette[8 + css->PRST], palette[10 + css->SEGT]);
	printf("SS =%04X %016llX:%05lX TYPE=%X DPL=%u [%c%c%c%c%c%c]\n", ss, ssa, ssl, (WORD) (SYSTEM_TABLE->GDT[ss >> 3] >> 40) & 0xF, sss->DPLX,palette[sss->GRAN], palette[2 + sss->SIZE], palette[4 + sss->LONG], palette[6 + (sss->RSV2 >> 4)], palette[8 + sss->PRST], palette[10 + sss->SEGT]);
	printf("DS =%04X %016llX:%05lX TYPE=%X DPL=%u [%c%c%c%c%c%c]\n", ds, dsa, dsl, (WORD) (SYSTEM_TABLE->GDT[ds >> 3] >> 40) & 0xF, dss->DPLX,palette[dss->GRAN], palette[2 + dss->SIZE], palette[4 + dss->LONG], palette[6 + (dss->RSV2 >> 4)], palette[8 + dss->PRST], palette[10 + dss->SEGT]);
	printf("FS =%04X %016llX:%05lX TYPE=%X DPL=%u [%c%c%c%c%c%c]\n", fs, fsa, fsl, (WORD) (SYSTEM_TABLE->GDT[fs >> 3] >> 40) & 0xF, fss->DPLX,palette[fss->GRAN], palette[2 + fss->SIZE], palette[4 + fss->LONG], palette[6 + (fss->RSV2 >> 4)], palette[8 + fss->PRST], palette[10 + fss->SEGT]);
	printf("GS =%04X %016llX:%05lX TYPE=%X DPL=%u [%c%c%c%c%c%c]\n", gs, gsa, gsl, (WORD) (SYSTEM_TABLE->GDT[gs >> 3] >> 40) & 0xF, gss->DPLX,palette[gss->GRAN], palette[2 + gss->SIZE], palette[4 + gss->LONG], palette[6 + (gss->RSV2 >> 4)], palette[8 + gss->PRST], palette[10 + gss->SEGT]);
	printf("TR =%04X %016llX:%05lX TYPE=%X DPL=%u [%c%c%c%c%c%c]\n", tr, tsa, tsl, tss->TYPE, tss->DPLX, palette[tss->GRAN], palette[2 + (tss->ZRO2 >> 1)], palette[4 + (tss->ZRO2 & 1)], palette[6 + tss->AVAL], palette[8 + tss->PRST], palette[10 + tss->ZRO0]);
	printf("LDT=%04X %016llX:%05lX TYPE=%X DPL=%u [%c%c%c%c%c%c]\n", lr, lsa, lsl, lss->TYPE, lss->DPLX, palette[lss->GRAN], palette[2 + (lss->ZRO2 >> 1)], palette[4 + (lss->ZRO2 & 1)], palette[6 + lss->AVAL], palette[8 + lss->PRST], palette[10 + lss->ZRO0]);
	printf("GDT=     %016llX:%05X\n", gdtr.A, gdtr.L);
	printf("IDT=     %016llX:%05X\n", *((QWORD *) idtr.Base), idtr.Limit);
	DWORD rflx = __getrfl();
	RFLAGS *rfl = (RFLAGS *) &rflx;
	char rflm[] = "-C-P-A-Z-S-T-I-D-O";
	printf("RFL=%08lX [%c%c%c%c%c%c%c%c%c] IOPL=%u", rflx,
		rflm[ 0 + rfl->CF],
		rflm[ 2 + rfl->PF],
		rflm[ 4 + rfl->AF],
		rflm[ 6 + rfl->ZF],
		rflm[ 8 + rfl->SF],
		rflm[10 + rfl->TF],
		rflm[12 + rfl->IF],
		rflm[14 + rfl->DF],
		rflm[16 + rfl->OF],
		rfl->IOPL
	);
	if (rflx & RFL_NT)
		printf(" NT");
	if (rflx & RFL_RF)
		printf(" RF");
	if (rflx & RFL_VM)
		printf(" VM");
	if (rflx & RFL_AC)
		printf(" AC");
	if (rflx & RFL_VIF)
		printf(" VIF");
	if (rflx & RFL_VIP)
		printf(" VIP");
	if (rflx & RFL_ID)
		printf(" ID");
	outchar('\n');
	DWORD efer = __rdmsr(MSR_EFER);
	printf("EFER=%08lX", efer);
	if (efer & EFER_SCE)
		printf(" SCE");
	if (efer & EFER_LME)
		printf(" LME");
	if (efer & EFER_LMA)
		printf(" LMA");
	if (efer & EFER_NXE)
		printf(" NXE");

	while (SYSTEM_TABLE->RUNN) _mm_pause();
}