
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "core/esma_ring_buffer.h"

struct test {
	int a;
	int b;
	int c;
};

int main()
{
	struct esma_ring_buffer rb;
	   int err;

	err = esma_ring_buffer_init(&rb, sizeof(struct test), 8);
	if (err) {
		printf("esma_ring_buffer_init() - failed\n");
		return 1;
	}

	for (int i = 0; i < 20; i++) {
		struct test *test = esma_ring_buffer_put(&rb);

		test->a = i;
		test->b = i + 1;
		test->c = i + 2;
	}

	while(1) {

		struct test *test;

		test = esma_ring_buffer_get(&rb);
		if (NULL == test)
			break;

		printf("\n");
		printf("test->a = %d\n", test->a);
		printf("test->b = %d\n", test->b);
		printf("test->c = %d\n", test->c);
		printf("\n");
	}

	return 0;
}
