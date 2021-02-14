
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

#include "esma_echo_server.h"

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

struct esma_socket server_socket = {0};
u16 port;

void init_server_socket()
{
	int err;

	err = esma_socket_init(&server_socket);
	if (err) {
		esma_user_log_err("%s() - can't init server_socket\n", __func__);
		goto __fail;
	}

	err = esma_socket_create(&server_socket, AF_INET);
	if (err) {
		esma_user_log_err("%s() - esma_socket_create('AF_INET'): failed\n", __func__);
		goto __fail;
	}

	err = esma_socket_reset(&server_socket);
	if (err) {
		esma_user_log_err("%s() - esma_socket_reset(): failed\n", __func__);
		goto __fail;
	}

	err = esma_socket_bind(&server_socket, port, NULL);
	if (err) {
		esma_user_log_err("%s() - esma_socket_bind('%d'): failed\n", __func__, port);
		goto __fail;
	}

	printf("port number: %hu\n", port);
	printf("socket fd: %d\n", server_socket.fd);
	printf("socket family: %d\n", server_socket.family);
	return;

__fail:
	esma_user_log_ftl("%s() - fatal error\n", __func__);
	exit(1);
}

#define LOGGING_FLAGS	ESMA_LOG_USER		|\
			ESMA_LOG_CORE		|\
			ESMA_LOG_ENGINE		|\
			ESMA_LOG_DISPATCHER

int main(int argc, char **argv)
{
	int err;
	int ret;
	pid_t pid = getpid();

	struct manager_ctx ctx = {0};

	if (argc < 2)
		usage();

	sscanf(argv[1], "%hu", &port);
	if (0 == port) {
		esma_user_log_ftl("%Invalud port number: %s\n", argv[1]);
		usage();
	}
	
	init_server_socket();
	
	esma_logger_set_log_level(ESMA_LOG_DBG);
	esma_logger_set_log_flags(LOGGING_FLAGS);

	/* Engine setting: enter */
	esma_engine_set_number_of_engines(1);
	err = esma_engine_init(0);
	if (err) {
		esma_user_log_ftl("esma_engine_init(0): failed\n", "");
		exit(1);
	}
	/* Engine setting: leave */

	/* Esma listener setting: enter */
	err = esma_listener_init(&listener, "listener", NULL, 0);
	if (err) {
		esma_user_log_ftl("esma_listener_init(): failed\n", "");
		exit(1);
	}
	/* Esma listener setting: leave */

	/* Esma manager setting: enter */
	err = esma_template_init(&manager_tmpl, "manager");
	if (err) {
		esma_user_log_ftl("Can't init manager template\n", "");
		exit(1);
	}

	err = esma_template_set_by_path(&manager_tmpl, "manager.esma");
	if (err) {
		esma_user_log_ftl("can't set 'manager' template\n", "");
		exit(1);
	}

	err = esma_init(&manager, "manager", &manager_tmpl, 0);
	if (err) {
		esma_user_log_ftl("can't init esma 'manager'\n", "");
		exit(1);
	}

	ctx.listener = &listener;
	ctx.socket = &server_socket;
	/* Esma manager setting: leave */

	/* Run machines: enter */
	esma_run(&listener, &manager);
	esma_run(&manager, &ctx);
	/* Run machines: leave */

	esma_user_log_nrm("Start proccess with pid '%d'\n", pid);
	while (1) {
		ret = esma_engine_exec(0);
		if (ret)
			break;

		esma_engine_wait();
	}

	return 0;
}
