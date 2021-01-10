
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "core/esma_murmur_hash.h"

void hash(u8 *data, u32 len)
{
	int ret = esma_murmur_hash(data, len);
	printf("%s() data: %s\t; hash: %d\n", __func__, (char *) data, ret);
}

int main()
{
	hash((u8 *) "hello", strlen("hello"));
	hash((u8 *) "Hello1", strlen("hello1"));
	return 0;
}
