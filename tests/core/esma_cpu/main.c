
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "core/esma_cpu.h"

static void print_info(void)
{
	printf("cpu_cacheline_size:	%d\n", cpu_cacheline_size);
	printf("cpu_supported_sse:	%s\n", cpu_supported_sse == 1 ? "yes" : "no");
	printf("cpu_supported_sse2:	%s\n", cpu_supported_sse2 == 1 ? "yes" : "no");
	printf("cpu_supported_sse3:	%s\n", cpu_supported_sse3 == 1 ? "yes" : "no");
	printf("cpu_supported_ssse3:	%s\n", cpu_supported_ssse3 == 1 ? "yes" : "no");
	printf("cpu_supported_sse4_1:	%s\n", cpu_supported_sse4_1 == 1 ? "yes" : "no");
	printf("cpu_supported_sse4_2:	%s\n", cpu_supported_sse4_2 == 1 ? "yes" : "no");	
}

#define SLEEP_TIME	(10000000)

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

	printf("\n***** CPU INFO *****\n");
	print_info();	

	return 0;
}
