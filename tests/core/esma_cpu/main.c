
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "core/esma_cpu.h"

void print_cpuinfo(void)
{
	printf("\n***** CPU INFO *****\n");
	printf("brand string:		%s\n", cpuinfo.brand_string);
	printf("vendor string:		%s\n", cpuinfo.vendor_string);
	printf("\n");
	printf("supported sse:		%s\n", cpuinfo.sse    ? "yes" : "no");
	printf("supported sse2:		%s\n", cpuinfo.sse2   ? "yes" : "no");
	printf("supported sse3:		%s\n", cpuinfo.sse3   ? "yes" : "no");
	printf("supported ssse3:	%s\n", cpuinfo.ssse3  ? "yes" : "no");
	printf("supported sse4_1:	%s\n", cpuinfo.sse4_1 ? "yes" : "no");
	printf("supported sse4_2:	%s\n", cpuinfo.sse4_2 ? "yes" : "no");
	printf("\n");
	printf("base frequency:		%d\n", cpuinfo.base_frq);
	printf("max frequency:		%d\n", cpuinfo.max_frq);
	printf("bus frequency:		%d\n", cpuinfo.bus_frq);

	printf("\n*** CACHE L1 ***\n");
	printf("data l1 cache size in kbs:		%d\n", cpuinfo.l1_data.l1.size_in_kbs);
	printf("data l1 cache associativity:		%d\n", cpuinfo.l1_data.l1.associativity);
	printf("data l1 cache lines per tag:		%d\n", cpuinfo.l1_data.l1.lines_per_tag);
	printf("data l1 cache line size in bytes:	%d\n", cpuinfo.l1_data.l1.line_size_in_bytes);
	printf("\n");
	printf("code l1 cache size in kbs:		%d\n", cpuinfo.l1_code.l1.size_in_kbs);
	printf("code l1 cache associativity:		%d\n", cpuinfo.l1_code.l1.associativity);
	printf("code l1 cache lines per tag:		%d\n", cpuinfo.l1_code.l1.lines_per_tag);
	printf("code l1 cache line size in bytes:	%d\n", cpuinfo.l1_code.l1.line_size_in_bytes);
	printf("\n*** CACHE L2 ***\n");
	printf("unified l2 cache size in kbs:		%d\n", cpuinfo.l2.l2.size_in_kbs);
	printf("unified l2 cache associativity:		%d\n", cpuinfo.l2.l2.associativity);
	printf("unified l2 cache lines per tag:		%d\n", cpuinfo.l2.l2.lines_per_tag);
	printf("unified l2 cache line size in bytes:	%d\n", cpuinfo.l2.l2.line_size_in_bytes);
	printf("\n*** CACHE L3 ***\n");
	printf("unified l3 cache size in 512 KB chunks:	%d\n", cpuinfo.l3.l3.size_in_512kb_chunks);
	printf("unified l3 cache associativity:		%d\n", cpuinfo.l3.l3.associativity);
	printf("unified l3 cache lines per tag:		%d\n", cpuinfo.l3.l3.lines_per_tag);
	printf("unified l3 cache line size in bytes:	%d\n", cpuinfo.l3.l3.line_size_in_bytes);
	printf("\n");
	printf("current core clock frequency (MHz):	%d\n", cpu_current_core_clock_frequency());
	printf("current core clock voltage (mV):	%d\n", cpu_current_core_clock_voltage());
	printf("current (LongRun) performance level:	%d\n", cpu_current_preformance_level());
	printf("current gate delay (fs):		%d\n", cpu_current_gate_delay());
	printf("\n");
	printf("mystery string:				%s\n", cpuinfo.mystery_string);
}

#define SLEEP_TIME	(1000000)

int main()
{
	u64 start;
	u64 end;

	start = esma_rdtscp();
	esma_mfence();

	usleep(SLEEP_TIME);
	
	esma_mfence();
	end = esma_rdtscp();

	printf("rdtsc: start ticks:	%ld\n", start);
	printf("rdtsc: end ticks:	%ld\n", end);
	printf("rdtsc: cnt ticks:	%ld\n", end - start);
	printf("rdtsc: frequency:	%ld\n", (end - start) / SLEEP_TIME);

	print_cpuinfo();

	return 0;
}
