
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "core/esma_array.h"

struct test {
	int a;
	int b;
	int c;
};

int main()
{
	struct esma_array array = {0};
	   int err;

	err = esma_array_init(&array, 5, sizeof(struct test));
	if (err) {
		printf("esma_array_init() - failed\n");
		return 1;
	}

	for (int i = 2; i < 10; i++) {
		struct test *t = esma_array_push(&array);

		t->a = i;
		t->b = i + 1;
		t->c = i + 2;
	}

	for (int i = 2; i < 10; i++) {
		struct test *t = esma_array_n(&array, i);
		printf("t->a = %d\n", t->a);
		printf("t->b = %d\n", t->b);
		printf("t->c = %d\n\n", t->c);
	}

	printf("The end.\n");
	esma_array_free(&array);
	return 0;
}
