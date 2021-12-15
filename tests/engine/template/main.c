
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "engine/esma.h"
#include "core/esma_logger.h"

int main()
{
	struct esma_template tmpl;
	   int err;

	esma_logger_set_log_level(ESMA_LOG_DBG);
	esma_logger_set_log_flags(ESMA_LOG_ENGINE);

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
