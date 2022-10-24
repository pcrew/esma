
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "core/esma_alloc.h"
#include "core/esma_array.h"
#include "core/esma_logger.h"
#include "core/esma_backtrace.h"

#include "common/api.h"

#include "engine/esma.h"

struct esma_template tmpl;
struct esma_engine engine;

#define LOGGING_FLAGS	ESMA_LOG_USER 		|\
			ESMA_LOG_CORE 		|\
			ESMA_LOG_ENGINE		|\
			ESMA_LOG_REACTOR	|\
			ESMA_LOG_DISPATCHER

void usage()
{
	printf("Run: ./test PORT\n");
	printf("Where PORT is port number\n");
	printf("For example: ./test 1771\n");
	exit(1);
}

int main(int argc, char **argv)
{
	int err;
	int ret;
	u16 port;
	pid_t pid = getpid();

	if (argc < 2)
		usage();

	sscanf(argv[1], "%hu", &port);
	if (0 == port) {
		printf("Invalid port: %s\n", argv[1]);
		usage();
	}

	esma_logger_set_log_level(ESMA_LOG_DBG);
	esma_logger_set_log_flags(LOGGING_FLAGS);

	err = esma_backtrace_init(32);
	if (err) {
		esma_user_log_err("esma_backtrace_init(32): failed.\n", "");
		return 1;
	}

	err = esma_template_init(&tmpl, "master");
	if (err) {
		esma_user_log_ftl("esma_template_init('master'): failed\n", "");
		return 1;
	}

	err = esma_template_set_by_path(&tmpl, "master.esma");
	if (err) {
		esma_user_log_ftl("esma_template_set('master'): failed\n", "");
		return 1;
	}

	err = esma_engine_init(&engine, "reactor_poll", "msg_queue_ring_buffer", ESMA_QUEUE_WITHOUT_LOCK);
	if (err) {
		esma_user_log_ftl("esma_engine_init(0): failed\n", "");
		return 1;
	}

	esma_user_log_nrm("procces '%d' has been started\n", pid);

	esma_machine_run(esma_machine_new(&engine, &tmpl, "master"), &port);

	while (1) {
		ret = esma_engine_exec(&engine);
		if (ret)
			break;

		esma_engine_wait(&engine);
	}

	return 0;
}
