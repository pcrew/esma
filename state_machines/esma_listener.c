
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "esma_listener.h"
#include "esma_sm_data.h"

#include "core/esma_dbuf.h"
#include "core/esma_alloc.h"
#include "core/esma_logger.h"

#define TO_IDLE	0
#define TO_WORK	0
#define TO_FINI	3

#define LISTEN_SUCCESS	1
#define LISTEN_FAILURE	2

char *esma_listener_tmpl = 
	"	states {						"
	"		work;						"
	"		idle;						"
	"	};							"
	"								"
	"	trans {							"
	"		init -> self: 1;				"
	"		init -> work: 0;				"
	"		init -> fini: 3;				"
	"								"
	"		work -> self: data_0: ESMA_POLLIN;		"
	"		work -> self: data_1: ESMA_POLLERR;		"
	"		work -> self: 1;				"
	"		work -> idle: 0;				"
	"								"
	"		idle -> self: 1;				"
	"		idle ->	work: 0;				"
	"		idle -> fini: 3;				"
	"	};							"
;

/* Public functions */
int esma_listener_init(struct esma *listener, char *name, char *tmpl_path, int ngn_id)
{
	struct esma_dbuf esma_listener_tmpl_dbuf;
	struct esma_template tmpl;
	   int err;

	esma_dbuf_set(&esma_listener_tmpl_dbuf, esma_listener_tmpl);

	if (NULL == name)
		name = "nameless";

	if (ngn_id < 0) {
		esma_user_log_err("%s()/%s - ngn_id < 0\n", __func__, name);
		return 1;
	}

	if (NULL == listener) {
		esma_user_log_err("%s()/%s - esma is NULL\n", __func__, name);
		return 1;
	}

	if (NULL == tmpl_path) {
		goto __set_basic;
	}

	goto __set_custom;

__set_basic:

	esma_user_log_dbg("%s()/%s - basic machine\n", __func__, name);
	err = esma_template_set_by_dbuf(&tmpl, &esma_listener_tmpl_dbuf);
	if (err) {
		esma_user_log_err("%s()/%s: set esma_template basic: failed\n",
				__func__, name);
		return 1;
	}

	goto __init;

__set_custom:

	esma_user_log_dbg("%s()/%s - custom machine\n", __func__, name);
	err = esma_template_set_by_path(&tmpl, tmpl_path);
	if (err) {
		esma_user_log_err("%s()/%s: esma_template_set('%s'): failed\n",
				__func__, tmpl_path);
		return 1;
	}

	goto __init;

__init:

	err = esma_init(listener, name, &tmpl, ngn_id);
	if (err) {
		esma_user_log_err("%s()/%s: esma_init(): failed\n",
				__func__, name);
		return 1;
	}

	esma_user_log_dbg("%s()/%s - machine successfuly inited\n", __func__, name);
	return 0;	
}

void esma_listener_run(struct esma *listener)
{
	if (NULL == listener) {
		esma_user_log_ftl("%s() - listener is NULL\n", __func__);
		exit(1);
	}

	esma_run(listener, NULL);
}

/* State machine actions */

#define __unbox__	struct esma *master, struct esma *me, void *dptr

struct listener_info {
	struct esma		*master;
	struct esma_socket	*server;
	struct esma_mempool	*socket_pool;
};

int esma_listener_init_enter(__unbox__)
{
	struct listener_info *li = NULL;
	
	li = esma_malloc(sizeof(struct listener_info));
	if (NULL == li) {
		esma_user_log_err("%s()/%s - can't allocate memory for info section\n",
				__func__, me->name);
		return 1;
	}

	li->master = master;

	me->data = li;
	return 0;
}

static int _listener_init(struct esma *me, struct esma *master, void *dptr)
{
	struct listener_info *li = me->data;
	struct esma_listener_context *ctx = dptr;
	   int err = 0;

	esma_engine_free_io_channel(me);

	if (NULL == ctx) {
		esma_user_log_err("%s()/%s - invalid context from %s\n",
				__func__, me->name, master->name);

		return 1;
	}

	if (esma_socket_invalid(ctx->server)) {
		esma_user_log_err("%s()/%s - invalid socket from %s\n",
				__func__,  me->name, master->name);
		return 1;
	}

	if (NULL == ctx->socket_pool) {
		err = esma_mempool_init(ctx->socket_pool, sizeof(struct esma_socket), MEMPOOL_SMALL_BLOCK);
	}

	if (err) {
		esma_user_log_err("%s()/%s - can't create socket mempool\n",
				__func__, me->name);
		return 1;
	}

	li->server = ctx->server;
	li->socket_pool = ctx->socket_pool;

	return 0;

}

/* message: get context and start working */
int esma_listener_init_1(__unbox__)
{
	int err;

	err = _listener_init(me, master, dptr);
	if (err) {
		esma_msg(me, me, NULL, TO_IDLE);
		esma_msg(me, master, NULL, LISTEN_FAILURE);
		return 0;
	}

	esma_msg(me, me, NULL, TO_WORK);
	return 0;
}

int esma_listener_init_leave(__unbox__)
{
	return 0;
}

int esma_listener_work_enter(__unbox__)
{
	struct listener_info *li = me->data;
	
	esma_engine_init_io_channel(me, li->server->fd);
	esma_engine_mod_io_channel(me, ESMA_POLLIN, IO_EVENT_ENABLE);
	me->io_channel.flags |= ESMA_LISTENING_CHANNEL;
	
	return 0;
}

/* In this action we always sand SUCCESS message for master. But, if we can't accept new client, then dptr is NULL;
 * esle dptr is client socket. */
int esma_listener_work_data_0(__unbox__)
{
	struct listener_info *li = me->data;
	struct esma_socket *client;
	   int err;

	esma_user_log_dbg("%s()/%s - new client\n", __func__, me->name);
	client = esma_mempool_get_block(li->socket_pool);
	if (NULL == client) {
		esma_user_log_wrn("%s()/%s - can't get block from socket mempool\n",
				__func__, me->name);

		goto __ret;
	}

	err = esma_socket_accept(client, li->server);
	if (err) {
		esma_user_log_wrn("%s()/%s - can't accept new client\n",
				__func__, me->name);

		esma_mempool_put_block(li->socket_pool, client);
		goto __ret;
	}
	esma_user_log_dbg("%s()/%s - new client successfuly accepted\n",
			__func__, me->name);

	esma_msg(me, li->master, client, LISTEN_SUCCESS);
	return 0;

__ret:
	esma_msg(me, li->master, NULL, LISTEN_SUCCESS);
	return 0;
}

int esma_listener_work_data_1(__unbox__)
{
	struct listener_info *li = me->data;

	esma_user_log_err("%s()/%s - have fail with socket.\n",
			__func__, me->name);

	esma_engine_mod_io_channel(me, 0, IO_EVENT_DISABLE);
	esma_engine_free_io_channel(me);

	esma_msg(me, me, NULL, TO_IDLE);
	esma_msg(me, li->master, NULL, LISTEN_FAILURE);
	return 0;
}

/* message: stop listening and go to idle */
int esma_listener_work_1(__unbox__)
{
	esma_engine_mod_io_channel(me, 0, IO_EVENT_DISABLE);
	esma_engine_free_io_channel(me);

	esma_msg(me, me, NULL, TO_IDLE);
	return 0;
}

int esma_listener_work_leave(__unbox__)
{
	return 0;
}

int esma_listener_idle_enter(__unbox__)
{
	return 0;
}

/* message: get new context and start working */
int esma_listener_idle_1(__unbox__)
{
	int err;

	err = _listener_init(me, master, dptr);
	if (err) {
		esma_msg(me, master, NULL, LISTEN_FAILURE);
		return 0;
	}

	esma_msg(me, me, NULL, TO_WORK);
	return 0;
}
