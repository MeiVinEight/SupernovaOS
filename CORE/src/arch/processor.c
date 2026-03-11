#include <arch/processor.h>
#include <intrinsic.h>

void setup_processor()
{
	// Setup APIC
	//QWORD *apic = (QWORD *) 0xFEE00000;
	//apic[0x80 / 8] = 0x10000; // Enable APIC

	// CHECK SSE
	DWORD cpuid[4] = { 0 };
	__cpuid(cpuid, 1);
	QWORD sse = 0;
	sse |= cpuid[2] & (1 << 20); // SSE 4.2
	sse |= cpuid[2] & (1 << 19); // SSE 4.1
	sse |= cpuid[2] & (1 << 9);  // SSSE 3
	sse |= cpuid[2] & 1;         // SSE 3
	sse |= cpuid[3] & (1 << 26); // SSE 2
	sse |= cpuid[3] & (1 << 25); // SSE
	if (sse)
	{
		// ENABLE SSE
		//__writecr0((__readcr0() & (~4ULL)) | 2); // CLEAR CR0.EM AND SET CR0.MP
		//__writecr4(__readcr4() | (3 << 9)); // SET CR4.OSFXSR AND CR4.OSXMMEEXCPT

		QWORD CR0 = __readcr0();
		// CR0.EM = 0
		CR0 &= ~4;
		// CR0.MP = 1
		CR0 |= 2;
		__writecr0(CR0);
		QWORD CR4 = __readcr4();
		// CR4.OSFXSR = 1
		// CR4.OSXMMEXCPT = 1
		CR4 |= 0x600;
		__writecr4(CR4);
	}
}