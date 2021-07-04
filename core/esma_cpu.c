
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

struct esma_cpuinfo cpuinfo = {0};

#if (__amd64__)
static struct cpu _cpuid(u32 info)
{
	struct cpu cpu = {0};

	__asm__(
		"cpuid"
		: "=a" (cpu.eax), "=b" (cpu.ebx), "=c" (cpu.ecx), "=d" (cpu.edx)
		: "a" (info)
		: "memory"
	);

	return cpu;
}
#endif

#if (__linux__)
__attribute__((constructor))
#endif
void esma_cpuid(void)
{

	struct cpu cpu;
	/* Getting vendor info */
	cpu = _cpuid(0);
	memcpy(cpuinfo.vendor_string, &cpu.ebx, sizeof(u32));
	memcpy(cpuinfo.vendor_string + 4, &cpu.edx, sizeof(u32));
	memcpy(cpuinfo.vendor_string + 8, &cpu.ecx, sizeof(u32));
	cpuinfo.vendor_string[12] = 0;
	/* */

	/* Getting features */
	cpu = _cpuid(1);
	cpuinfo.sse	= cpu.edx & (1 << 25) || 0;
	cpuinfo.sse2	= cpu.edx & (1 << 26) || 0;
	cpuinfo.sse3	= cpu.ecx & (1 << 0)  || 0;
	cpuinfo.ssse3	= cpu.ecx & (1 << 9)  || 0;
	cpuinfo.sse4_1	= cpu.ecx & (1 << 19) || 0;
	cpuinfo.sse4_2	= cpu.ecx & (1 << 20) || 0;
	/*  */

	/* Getting frequency info */
	cpu = _cpuid(0x00000015);
	cpuinfo.base_frq = cpu.eax & 0x0000FFFF;
	cpuinfo.max_frq = cpu.ebx & 0x0000FFFF;
	cpuinfo.bus_frq = cpu.ecx & 0x0000FFFF;

	/* Getting brand string */
	cpu = _cpuid(0x80000002);
	memcpy(cpuinfo.brand_string, &cpu, sizeof(struct cpu));
	cpu = _cpuid(0x80000003);
	memcpy(cpuinfo.brand_string + 16, &cpu, sizeof(struct cpu));
	cpu = _cpuid(0x80000004);
	memcpy(cpuinfo.brand_string + 32, &cpu, sizeof(struct cpu));
	cpuinfo.brand_string[48] = 0;
	/*  */

	/* Getting cache info */
	cpu = _cpuid(0x80000005);
	cpuinfo.l1_data.reg = cpu.ecx;
	cpuinfo.l1_code.reg = cpu.edx;

	cpu = _cpuid(0x80000006);
	cpuinfo.l2.reg = cpu.ecx;
	cpuinfo.l3.reg = cpu.edx;
	/*  */

	/* mystery */
	cpu = _cpuid(0x8FFFFFFF);
	memcpy(cpuinfo.mystery_string, &cpu, sizeof(struct cpu));
	cpuinfo.mystery_string[15] = 0;
	/*  */

	cpuinfo.status = 1;
}

u32 cpu_current_core_clock_frequency(void)
{
	struct cpu cpu = _cpuid(0x80860007);
	return cpu.eax;
}

u32 cpu_current_core_clock_voltage(void)
{
	struct cpu cpu = _cpuid(0x80860007);
	return cpu.ebx;
}

u32 cpu_current_preformance_level(void)
{
	struct cpu cpu = _cpuid(0x80860007);
	return cpu.ecx;
}

u32 cpu_current_gate_delay(void)
{
	struct cpu cpu = _cpuid(0x80860007);
	return cpu.edx;
}
