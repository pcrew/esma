
#include <stdlib.h>
#include <string.h>

#include "esma_cpu.h"
#include "common/numeric_types.h"

//u32 cpu_vendor_info[5] = {0, 0, 0, 0, 0};
u32 cpu_info[4] = {0, 0, 0, 0};

u32 cpu_base_frequency;
u32 cpu_max_frequency;
u32 cpu_bus_frequency;

u32 cpu_cacheline_size = 0;

u32 cpu_supported_sse = 0;
u32 cpu_supported_sse2 = 0;
u32 cpu_supported_sse3 = 0;
u32 cpu_supported_sse4_1 = 0;
u32 cpu_supported_sse4_2 = 0;
u32 cpu_supported_ssse3 = 0;

#define EAX	0
#define EBX	1
#define EDX	2
#define ECX	3

#if !__ARM__
static void _cpuid(u32 *buf, u32 info)
{
	__asm__(
		"mov    %%ebx,	%%esi;	"
		"cpuid;			"
		"mov	%%eax,	  (%1);	"
		"mov	%%ebx,	 4(%1);	"
		"mov	%%edx,	 8(%1);	"
		"mov	%%ecx,	12(%1);	"
		"mov	%%esi,	 %%ebx;	"
		: /* void */
		: "a" (info), "D" (buf)
		: "ecx", "edx", "esi", "memory"
	);
}
#endif

#if (__linux__)
__attribute__((constructor))
#endif
void esma_cpuid(void)
{
	_cpuid(cpu_info, 0);

	if (0 == strcmp((char*) &cpu_info[1], "AuthenticAMD")) {
		cpu_cacheline_size = 64;
	}

	_cpuid(cpu_info, 1);

	cpu_supported_sse	= cpu_info[EDX] & (1 << 25) || 0;
	cpu_supported_sse2	= cpu_info[EDX] & (1 << 26) || 0;
	cpu_supported_sse3	= cpu_info[ECX] & (1 << 0) || 0;
	cpu_supported_ssse3	= cpu_info[ECX] & (1 << 9) || 0;
	cpu_supported_sse4_1	= cpu_info[ECX] & (1 << 19) || 0;
	cpu_supported_sse4_2	= cpu_info[ECX] & (1 << 20) || 0;
}
