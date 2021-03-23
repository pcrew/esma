
#include <stdlib.h>
#include <unistd.h>

#include "core/esma_alloc.h"
#include "core/esma_socket.h"
#include "core/esma_logger.h"
#include "core/esma_mempool.h"
#include "core/esma_objpool.h"

#include "engine/esma_engine.h"

#include "common/compiler.h"
#include "common/numeric_types.h"

#include "state_machines/esma_state_machines.h"

#include "esma_echo_server.h"

#define __unbox__	struct esma *slave, struct esma *me, void *dptr

struct manager_info {
	struct esma *listener;	
	struct esma_listener_context l_ctx;

	struct esma_mempool state_machines;

	struct esma_objpool worker_pool;
	struct esma_objpool rx_pool;
	struct esma_objpool tx_pool;
};

static int _manager_init_and_start_machines(struct esma *manager)
{
	struct manager_info *mi = manager->data;
	struct esma_objpool *objpool;
	struct esma_mempool *mempool;
	   int err;

	if (unlikely(NULL == mi)) {
		esma_user_log_ftl("%s()/%s - data is NULL! You must assgiment me->data = (struct manager_info *) in 'manager_init_enter()'\n",
				__func__, manager->name);
		exit(1);
	}

	mempool = &mi->state_machines;
	err = esma_mempool_init(mempool, sizeof(struct esma), MEMPOOL_SMALL_BLOCK);
	if (err) {
		esma_user_log_err("%s()/%s - can't init mempool for state machines.\n",
				__func__, manager->name);
		exit(1);
	}

#define NRX 32
	objpool = &mi->rx_pool;
	err = esma_objpool_init(objpool, 32, 0, 0);
	if (err) {
		esma_user_log_err("%s()/%s - can't init objpool for rx machines\n",
				__func__, manager->name);
		exit(1);
	}
	for (int i = 0; i < NRX; i++) {
		struct esma *rx = esma_mempool_get_block(mempool);
		  char name[64];

		if (NULL == rx) {
			esma_user_log_ftl("%s()/%s - can't get block from mempool\n",
					__func__, manager->name);
			exit(1);
		}

		err = esma_objpool_put(objpool, rx);
		if (err) {
			esma_user_log_ftl("%s()/%s - can't put rx '%s' to objpool\n",
					__func__, manager->name, name);
			exit(1);
		}

		sprintf(name, "RX_%d", i);
		esma_user_log_nrm("%s(): name: %s\n", __func__, name);

		err = esma_rx_init(rx, name, NULL, 0);
		if (err) {
			esma_user_log_ftl("%s()/%s - can't init RX state_machine\n",
					__func__, manager->name);
			exit(1);
		}

		esma_run(rx, objpool);
	}

#define NTX 32
	objpool = &mi->tx_pool;
	err = esma_objpool_init(objpool, 32, 0, 0);
	if (err) {
		esma_user_log_err("%s()/%s - can't init objpool for tx machines\n",
				__func__, manager->name);
		exit(1);
	}
	for (int i = 0; i < NTX; i++) {
		struct esma *tx = esma_mempool_get_block(mempool);
		  char name[64];

		if (NULL == tx) {
			esma_user_log_ftl("%s()/%s - can't get block from mempool\n",
					__func__, manager->name);
			exit(1);
		}

		err = esma_objpool_put(objpool, tx);
		if (err) {
			esma_user_log_ftl("%s()/%s - can't put TX '%s' to objpool\n",
					__func__, manager->name, name);
			exit(1);
		}
	
		sprintf(name, "TX_%d", i);
		esma_user_log_nrm("%s(): name: %s\n", __func__, name);

		err = esma_tx_init(tx, name, NULL, 0);
		if (err) {
			esma_user_log_ftl("%s()/%s - can't init TX state_machine\n",
					__func__, manager->name);
			exit(1);
		}

		esma_run(tx, objpool);
	}

#define NWORKERS	128
	objpool = &mi->worker_pool;
	err = esma_objpool_init(objpool, 128, 0, 0);
	if (err) {
		esma_user_log_err("%s()/%s - can't init objpool for worker machines\n",
				__func__, manager->name);
		exit(1);
	}

	for (int i = 0; i < NWORKERS; i++) {
		struct esma *worker = esma_mempool_get_block(mempool);
		  char name[64];

		if (NULL == worker) {
			esma_user_log_ftl("%s()/%s - can't get block from mempool\n",
					__func__, manager->name);
			exit(1);
		}

		err = esma_objpool_put(objpool, worker);
		if (err) {
			esma_user_log_ftl("%s()/%s - can't put TX '%s' to objpool\n",
					__func__, manager->name, name);
			exit(1);
		}
	
		sprintf(name, "WORKER_%d", i);
		esma_user_log_nrm("%s(): name: %s\n", __func__, name);

	}	

	return 0;	
}

int manager_init_enter(__unbox__)
{
	struct manager_info *mi;
	struct manager_ctx *ctx = dptr;
	   int err;

	if (NULL == ctx) {
		esma_user_log_err("%s()/%s - dptr is NULL\n", __func__, me->name);
		goto __fini;
	}

	mi = esma_malloc(sizeof(struct manager_info));
	if (NULL == mi) {
		esma_user_log_err("%s()/%s - can't allocate memory for private data\n",
				__func__, me->name);
		goto __fini;
	}

	mi->listener = ctx->listener;
	mi->l_ctx.server = ctx->socket;

	me->data = mi;
	
	esma_user_log_dbg("%s()/%s start initialize derived sate machines\n",
			__func__, me->name);

	err = _manager_init_and_start_machines(me);
	if (err) {
		esma_user_log_err("%s()/%s - _manager_init_and_start_machines(): failed\n",
				__func__, me->name);
		goto __fini;
	}

	esma_user_log_dbg("%s()/%s - successfuly started\n", __func__, me->name);
	esma_msg(me, me, NULL, 0);
	esma_msg(me, mi->listener, &mi->l_ctx, 1);
	return 0;

__fini:
	return 0;
}

int manager_init_leave(__unbox__)
{
	return 0;
}

int manager_idle_enter(__unbox__)
{
	return 0;
}

int manager_idle_1(__unbox__)
{
	return 0;
}

int manager_idle_2(__unbox__)
{
	return 0;
}

int manager_idle_leave(__unbox__)
{
	return 0;
}

int manager_work_enter(__unbox__)
{
	return 0;
}

int manager_work_1(__unbox__)
{
	return 0;
}

int manager_work_2(__unbox__)
{
	return 0;
}

int manager_work_leave(__unbox__)
{
	return 0;
}

int manager_fini_enter(__unbox__)
{
	return 0;
}

int manager_fini_leave(__unbox__)
{
	return 0;
}
