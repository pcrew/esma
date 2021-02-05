
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "esma_tx.h"
#include "esma_sm_data.h"

#include "core/esma_dbuf.h"
#include "core/esma_alloc.h"
#include "core/esma_logger.h"

#define TO_WORK	0
#define TO_IDLE	0

#define SEND_SUCCESS	1
#define SEND_FAILURE	2

char *esma_tx_tmpl = 
	"	states {						"
	"		idle;						"
	"		work;						"
	"	};							"
	"								"
	"	trans {							"
	"		init -> idle: 0;				"
	"		init -> fini: 3;				"
	"								"
	"		idle -> self: 1;				"
	"		idle -> work: 0;				"
	"								"
	"		work -> self: tick_0: 1000ms: ESMA_TM_ONESHOT;	"
	"		work -> self: data_0: ESMA_POLLOUT;		"
	"		work -> self: data_1: ESMA_POLLERR;		"
	"		work -> idle: 0;				"
	"	};							"
	"								"
;

/* Public functions */
int esma_tx_init(struct esma *tx, char *name, char *tmpl_path, int ngn_id)
{
	struct esma_dbuf esma_tx_tmpl_dbuf;
	struct esma_template tmpl;
	   int err;

	esma_dbuf_set(&esma_tx_tmpl_dbuf, esma_tx_tmpl);

	if (NULL == name)
		name = "nameless";

	if (ngn_id < 0) {
		esma_user_log_err("%s()/%s - ngn_id < 0\n", __func__, name);
		return 1;
	}

	if (NULL == tx) {
		esma_user_log_err("%s()/%s - esma is NULL\n", __func__, name);
		return 1;
	}

	if (NULL == tmpl_path) {
		goto __set_basic;
	}

	goto __set_custom;

__set_basic:

	err = esma_template_set_by_dbuf(&tmpl, &esma_tx_tmpl_dbuf);
	if (err) {
		esma_user_log_err("%s()/%s: set esma_template basic: failed\n",
				__func__, name);
		return 1;
	}

	goto __init;

__set_custom:

	err = esma_template_set_by_path(&tmpl, tmpl_path);
	if (err) {
		esma_user_log_err("%s()/%s: esma_template_set('%s'): failed\n",
				__func__, tmpl_path);
		return 1;
	}

	goto __init;

__init:

	err = esma_init(tx, name, &tmpl, ngn_id);
	if (err) {
		esma_user_log_err("%s()/%s: esma_init(): failed\n",
				__func__, name);
		return 1;
	}

	esma_user_log_dbg("%s()/%s - machine successfuly inited\n", __func__, name);
	return 0;	
}

void esma_tx_run(struct esma *tx, struct esma_objpool *restroom)
{
	if (NULL == tx) {
		esma_user_log_ftl("%s() - tx is NULL\n", __func__);
		exit(1);
	}

	esma_run(tx, restroom);
}

/* State machine actions */

#define __unbox__	struct esma *requester, struct esma *me, void *dptr

struct tx_info {
	struct esma		*requester;
	struct esma_dbuf	*dbuf;
	struct esma_objpool	*restroom;
	struct esma_channel	*tick_waiting;
};

int esma_tx_init_enter(__unbox__)
{
	struct tx_info *txi = NULL;
	
	txi = esma_malloc(sizeof(struct tx_info));
	if (NULL == txi) {
		esma_user_log_err("%s()/%s - can't allocate memory for info section\n",
				__func__, me->name);
		return 1;
	}

	txi->restroom = dptr;
	txi->tick_waiting = esma_get_channel(me, "work", 0, ESMA_CH_TICK);
	if (NULL == txi->tick_waiting) {
		esma_user_log_wrn("%s()/%s - can't get channel 0 for 'work' state. Did you add tick_0 in esma file?\n",
				__func__, me->name);
	}

	esma_engine_free_io_channel(me);
	
	me->data = txi;
	esma_msg(me, me, NULL, TO_IDLE);
	return 0;
}

int esma_tx_init_leave(__unbox__)
{
	return 0;
}

int esma_tx_idle_enter(__unbox__)
{
	struct tx_info *txi = me->data;
	   int err;

	if (me->io_channel.fd) {
		esma_engine_mod_io_channel(me, 0, IO_EVENT_DISABLE);
	}

	esma_engine_free_io_channel(me);

	if (NULL == txi->restroom)
		return 0;
	
	err = esma_objpool_put(txi->restroom, me);
	if (err) {
		esma_user_log_ftl("%s()/%s - esma_objpool_put(): failed\n",
				__func__, me->name);
		exit(1);
	}

	return 0;
}

/* message from requester */
int esma_tx_idle_1(__unbox__)
{
	struct esma_io_context *ctx = dptr;
	struct tx_info *txi = me->data;

	if (NULL == ctx) {
		esma_user_log_wrn("%s()/%s - empty context from '%s'\n",
				__func__, me->name, requester->name);

		goto __fail;
	}

	if (ctx->fd <= 0) {
		esma_user_log_wrn("%s()/%s - invalid fd from '%s'\n",
				__func__, me->name, requester->name);
		goto __fail;
	}

	txi->requester = requester;

	txi->dbuf = ctx->dbuf;
	txi->dbuf->pos = txi->dbuf->loc;

	esma_dbuf_clear(txi->dbuf);
	
	esma_engine_init_io_channel(me, ctx->fd);
	esma_engine_mod_io_channel(me, ESMA_POLLOUT, IO_EVENT_ENABLE);
	
	esma_channel_set_interval(txi->tick_waiting, ctx->timeout_wait);

	esma_msg(me, me, NULL, TO_WORK); 
	return 0;

__fail:
	esma_msg(me, requester, NULL, SEND_FAILURE);
	return 0;
}

int esma_tx_idle_leave(__unbox__)
{
	return 0;
}

int esma_tx_work_enter(__unbox__)
{
	struct tx_info *txi = me->data;
	struct esma *req = txi->requester;
	struct esma_channel *ch = &requester->io_channel;

	esma_user_log_dbg("%s()/%s start sending to fd '%d' for '%s'\n",
			__func__, me->name, ch->fd, req->name);

	return 0;
}

int esma_tx_work_data_0(__unbox__)
{
	struct tx_info *txi = me->data;
	struct esma_dbuf *dbuf = txi->dbuf;
	   int n;

	n = write(me->io_channel.fd, dbuf->pos, dbuf->cnt);
	if (n < 0) {
		if (EINTR == errno)
			return 0;

		goto __send_fail;
	}

	if (n == dbuf->cnt) {
		esma_msg(me, me, NULL, TO_IDLE);
		esma_msg(me, requester, NULL, SEND_SUCCESS);
		return 0;
	}

	dbuf->pos += n;
	dbuf->cnt -= n;

	return 0;

__send_fail:

	esma_msg(me, me, NULL, TO_IDLE);
	esma_msg(me, requester, NULL, SEND_FAILURE);
	return 0;
}

int esma_tx_work_data_1(__unbox__)
{
	struct tx_info *txi = me->data;
	
	esma_user_log_dbg("%s()/%s - send: failed\n", __func__, me->name);
	esma_msg(me, me, NULL, TO_IDLE);
	esma_msg(me, txi->requester, NULL, SEND_FAILURE);

	return 0;
}

int esma_tx_work_tick_0(__unbox__)
{
	struct tx_info *txi = me->data;

	esma_user_log_dbg("%s()/%s - send: timeout\n", __func__, me->name);
	esma_msg(me, me, NULL, TO_IDLE);
	esma_msg(me, txi->requester, NULL, SEND_FAILURE);
	return 0;
}

int esma_tx_work_leave(__unbox__)
{
	return 0;
}
