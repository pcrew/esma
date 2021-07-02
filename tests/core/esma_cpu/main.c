
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "core/esma_cpu.h"

static void print_info(void)
{
	printf("brand_string:		%s\n", cpuinfo.brand_string);
	printf("vendor_string:		%s\n", cpuinfo.vendor_string);
	printf("cpuinfo.sse:		%s\n", cpuinfo.sse    ? "yes" : "no");
	printf("cpuinfo.sse2:		%s\n", cpuinfo.sse2   ? "yes" : "no");
	printf("cpuinfo.sse3:		%s\n", cpuinfo.sse3   ? "yes" : "no");
	printf("cpuinfo.ssse3:		%s\n", cpuinfo.ssse3  ? "yes" : "no");
	printf("cpuinfo.sse4_1:		%s\n", cpuinfo.sse4_1 ? "yes" : "no");
	printf("cpuinfo.sse4_2:		%s\n", cpuinfo.sse4_2 ? "yes" : "no");	
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

	printf("\n***** CPU INFO *****\n");
	print_info();	

	return 0;
}
