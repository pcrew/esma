/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "core/esma_time.h"
#include "core/esma_alloc.h"
#include "core/esma_logger.h"
#include "core/esma_mempool.h"
#include "core/esma_ring_buffer.h"

#include "utils/load_tool.h"

#include "common/compiler.h"

#include "esma.h"

extern int errno;

int esma_engine_init(struct esma_engine *ngn, char *reactor_name, char *msg_queue_name, u32 lock_type)
{
	struct reactor_ops *reactor_ops;
	struct msg_queue_ops *msg_queue_ops;
	int err;

	reactor_name = reactor_name == NULL ? "reactor_epoll" : reactor_name;
	msg_queue_name = msg_queue_name == NULL ? "msg_queue_ring_buffer" : msg_queue_name;

	reactor_ops = get_api(reactor_name);
	if (NULL == reactor_ops) {
		esma_engine_log_ftl("%s() - reactor '%s' doesn't exist.\n", __func__, reactor_name);
		exit(1);
	}

	msg_queue_ops = get_api(msg_queue_name);
	if (NULL == msg_queue_ops) {
		esma_engine_log_ftl("%s() - message queue '%s' doesn't exist.\n", __func__, msg_queue_name);
		exit(1);
	}

	ngn->reactor.ops = *reactor_ops;
	err = ngn->reactor.ops.init(&ngn->reactor.reactor, 32);
	if (err) {
		esma_engine_log_ftl("%s() - can't initialize reactor '%s'.\n", __func__, reactor_name);
		exit(1);
	}

	ngn->queue.ops = *msg_queue_ops;
	err = ngn->queue.ops.init(&ngn->queue.queue, lock_type);
	if (err) {
		esma_engine_log_ftl("%s() - can't initialize queue '%s'.\n", __func__, msg_queue_name);
		exit(1);
	}

	err = esma_mempool_init(&ngn->machine_pool, sizeof(struct esma));
	if (err) {
		esma_engine_log_ftl("%s() - can't initialize machine pool.\n", __func__);
		exit(1);
	}

	return 0;
}

int esma_engine_exec(struct esma_engine *ngn)
{
	struct esma_message *msg;
	   int ret = 0;

	esma_time_update();

	for (;;) {
		msg = ngn->queue.ops.get(&ngn->queue.queue);
		if (NULL == msg) {
			break;
		}

		ret = esma_engine_dispatcher_send(msg);
		if (ret) {
			break;
		}
	}

	return ret;
}

void esma_engine_wait(struct esma_engine *ngn)
{
	ngn->reactor.ops.wait(&ngn->reactor.reactor);
}
