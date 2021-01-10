
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "engine/esma_template.h"

int main()
{
	struct esma_template tmpl;
	   int err;

	err = esma_template_init(&tmpl, "test");
	if (err) {
		printf("esma_template_init() - failed\n");
		return 1;
	}

	err = esma_template_set_by_path(&tmpl, "test.esma");
	if (err) {
		printf("esma_template_set() - failed\n");
		return 0;
	}

	printf("\n\n\n*** RESULT ***\n");
	esma_template_print(&tmpl);
	return 0;
}
