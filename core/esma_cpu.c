
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esma_cpu.h"
#include "common/numeric_types.h"

struct cpu {
	u32 eax;
	u32 ebx;
	u32 ecx;
	u32 edx;
};

struct cpu cpu = {0};
struct esma_cpuinfo cpuinfo = {0};

#if (__amd64__)
static void _cpuid(u32 info)
{
	__asm__(
		"cpuid"
		: "=a" (cpu.eax), "=b" (cpu.ebx), "=c" (cpu.ecx), "=d" (cpu.edx)
		: "a" (info)
		: "memory"
	);
}
#endif

#if (__linux__)
__attribute__((constructor))
#endif
void esma_cpuid(void)
{
	/* Getting vendor info */
	_cpuid(0);
	memcpy(cpuinfo.vendor_string, &cpu.ebx, sizeof(u32));
	memcpy(cpuinfo.vendor_string + 4, &cpu.edx, sizeof(u32));
	memcpy(cpuinfo.vendor_string + 8, &cpu.ecx, sizeof(u32));
	cpuinfo.vendor_string[12] = 0;
	/* */

	/* Getting features */
	_cpuid(1);
	cpuinfo.sse	= cpu.edx & (1 << 25) || 0;
	cpuinfo.sse2	= cpu.edx & (1 << 26) || 0;
	cpuinfo.sse3	= cpu.ecx & (1 << 0)  || 0;
	cpuinfo.ssse3	= cpu.ecx & (1 << 9)  || 0;
	cpuinfo.sse4_1	= cpu.ecx & (1 << 19) || 0;
	cpuinfo.sse4_2	= cpu.ecx & (1 << 20) || 0;
	/*  */

	/* Getting brand string */
	_cpuid(0x80000002);
	memcpy(cpuinfo.brand_string, &cpu, sizeof(struct cpu));
	_cpuid(0x80000003);
	memcpy(cpuinfo.brand_string + 16, &cpu, sizeof(struct cpu));
	_cpuid(0x80000004);
	memcpy(cpuinfo.brand_string + 32, &cpu, sizeof(struct cpu));
	cpuinfo.brand_string[48] = 0;
	/*  */

	/* Getting cache info */
	_cpuid(0x80000005);
	cpuinfo.l1_cache_line_size = cpu.ecx & 0x000000FF; /* data L1 */
	_cpuid(0x80000006);
	cpuinfo.l2_cache_line_size = cpu.ecx & 0x000000FF;
	cpuinfo.l3_cache_line_size = cpu.edx & 0x000000FF;
	/*  */
}
