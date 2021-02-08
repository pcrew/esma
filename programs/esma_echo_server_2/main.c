
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "core/esma_alloc.h"
#include "core/esma_array.h"
#include "core/esma_logger.h"

#include "engine/esma_engine.h"

#include "state_machines/esma_sm_data.h"
#include "state_machines/esma_listener.h"

#include "worker.h"
#include "manager.h"

void usage()
{
	printf("Run: ./test PORT\n");
	printf("Where PORT is port number\n");
	printf("For example: ./test 1771\n");
	exit(1);
}

struct esma		manager;
struct esma		listener;

struct esma_template	manager_tmpl;

#define LOGGING_FLAGS	ESMA_LOG_USER		|\
			ESMA_LOG_CORE		|\
			ESMA_LOG_ENGINE		|\
			ESMA_LOG_DISPATCHER

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
		esma_user_log_ftl("%Invalud port number: %s\n", argv[1]);
		usage();
	}

	esma_logger_set_log_level(ESMA_LOG_DBG);
	esma_logger_set_log_flags(LOGGING_FLAGS);

	esma_engine_set_number_of_engines(1);
	err = esma_engine_init(0);
	if (err) {
		esma_user_log_ftl("esma_engine_init(0): failed\n", "");
		exit(1);
	}


	err = esma_template_init(&manager_tmpl, "manager");
	if (err) {
		esma_user_log_ftl("Can't init manager template\n", "");
		exit(1);
	}

	err = esma_listener_init(&listener, "listener", NULL, 0);
	if (err) {
		esma_user_log_ftl("esma_listener_init(): failed\n", "");
		exit(1);
	}

	esma_listener_run(&listener);
	while (1) {
		ret = esma_engine_exec(0);
		if (ret)
			break;

		esma_engine_wait();
	}

	return 0;
}
