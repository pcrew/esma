
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "esma_rx.h"
#include "esma_sm_data.h"

#include "core/esma_dbuf.h"
#include "core/esma_alloc.h"
#include "core/esma_logger.h"

#define TO_WORK	0
#define TO_IDLE	0
#define TO_DONE	1

#define RECV_SUCCESS	1
#define RECV_FAILURE	2

char *rx_tmpl = 
	"	states {						"
	"		idle;						"
	"		work;						"
	"		done;						"
	"	};							"
	"								"
	"	trans {							"
	"		init -> idle: 0;				"
	"		init -> fini: 3;				"
	"								"
	"		idle -> self: 1;				"
	"		idle -> work: 0;				"
	"								"
	"		work -> self: tick_0: 0ms: ESMA_TM_ONESHOT;	"
	"		work -> self: data_0: ESMA_POLLIN;		"
	"		work -> self: data_1: ESMA_POLLERR;		"
	"		work -> done: 1;				"
	"		work -> idle: 0;				"
	"								"
	"		done -> self: tick_0: 100ms: ESMA_TM_ONESHOT;	"
	"		done -> self: data_0: ESMA_POLLIN;		"
	"		done -> self: data_1: ESMA_POLLERR;		"
	"		done -> work: 1;				"	
	"		done -> idle: 0;				"
	"	};							"
	"								"
;

/* Public functions */
int esma_rx_init(struct esma *rx, char *name, char *tmpl_path, int ngn_id)
{
	struct esma_dbuf tmpl_dbuf;
	struct esma_template tmpl;
	   int err;

	esma_dbuf_set(&tmpl_dbuf, rx_tmpl);

	if (NULL == name)
		name = "nameless";

	err = esma_template_init(&tmpl, "esma_rx");
	if (err) {
		esma_user_log_err("%s()/%s - can't init template\n", __func__, name);
		return 1;
	}

	if (ngn_id < 0) {
		esma_user_log_err("%s()/%s - ngn_id < 0\n", __func__, name);
		return 1;
	}

	if (NULL == rx) {
		esma_user_log_err("%s()/%s - esma is NULL\n", __func__, name);
		return 1;
	}

	if (NULL == tmpl_path) {
		goto __set_basic;
	}

	goto __set_custom;

__set_basic:

	err = esma_template_set_by_dbuf(&tmpl, &tmpl_dbuf);
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

	err = esma_init(rx, name, &tmpl, ngn_id);
	if (err) {
		esma_user_log_err("%s()/%s: esma_init(): failed\n",
				__func__, name);
		return 1;
	}

	esma_user_log_dbg("%s()/%s - machine successfuly inited\n", __func__, name);
	return 0;	
}

void esma_rx_run(struct esma *rx, struct esma_objpool *restroom)
{
	if (NULL == rx) {
		esma_user_log_ftl("%s() - rx is NULL\n", __func__);
		exit(1);
	}

	esma_run(rx, restroom);
}

/* State machine actions */

#define __unbox__	struct esma *requester, struct esma *me, void *dptr

#define RX_IO_ENABLE		0
#define RX_IO_DISABLE		1

struct rx_info {
	struct esma		*requester;
	struct esma_dbuf	*dbuf;
	struct esma_objpool	*restroom;
	struct esma_channel	*tick_waiting;
	struct esma_channel	*tick_after_recv;
	u32			 timeout_after_recv;

	read_done_f		 read_done;
	u32			 status;
};

int _rx_done(struct esma *me, struct esma *requester, u32 code)
{
	struct rx_info *rxi = me->data;

	esma_engine_mod_io_channel(me, 0, IO_EVENT_DISABLE);
	rxi->status = RX_IO_DISABLE;

	esma_msg(me, me, NULL, TO_IDLE);
	esma_msg(me, requester, NULL, code);
	return 0;
}

int esma_rx_init_enter(__unbox__)
{
	struct rx_info *rxi = NULL;
	
	rxi = esma_malloc(sizeof(struct rx_info));
	if (NULL == rxi) {
		esma_user_log_err("%s()/%s - can't allocate memory for info section\n",
				__func__, me->name);
		return 1;
	}

	rxi->restroom = dptr;
	rxi->tick_waiting = esma_get_channel(me, "work", 0, ESMA_CH_TICK);
	if (NULL == rxi->tick_waiting) {
		esma_user_log_wrn("%s()/%s - can't get channel 0 for 'work' state. Did you add tick_0 for work state?\n",
				__func__, me->name);
	}

	rxi->tick_after_recv = esma_get_channel(me, "done", 0, ESMA_CH_TICK);
	if (NULL == rxi->tick_after_recv) {
		esma_user_log_wrn("%s()/%s - can't get channel 0 for 'done' state. Did you add tick_0 for done state?\n",
				__func__, me->name);
	}

	me->data = rxi;
	esma_msg(me, me, NULL, TO_IDLE);
	return 0;
}

int esma_rx_init_leave(__unbox__)
{
	return 0;
}

int esma_rx_idle_enter(__unbox__)
{
	struct rx_info *rxi = me->data;
	   int err;

	if (NULL == rxi->restroom)
		return 0;

	esma_engine_free_io_channel(me);
	
	err = esma_objpool_put(rxi->restroom, me);
	if (err) {
		esma_user_log_ftl("%s()/%s - esma_objpool_put(): failed\n",
				__func__, me->name);
		exit(1);
	}

	return 0;
}

/* message from requester */
int esma_rx_idle_1(__unbox__)
{
	struct esma_io_context *ctx = dptr;
	struct rx_info *rxi = me->data;

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

	rxi->requester = requester;
	rxi->dbuf = ctx->dbuf;
	rxi->timeout_after_recv = ctx->timeout_after_recv;
	rxi->read_done = ctx->read_done;
	rxi->status = RX_IO_DISABLE;

	esma_dbuf_clear(rxi->dbuf);
	
	esma_engine_init_io_channel(me, ctx->fd);
	
	esma_channel_set_interval(rxi->tick_waiting, ctx->timeout_wait);

	esma_msg(me, me, NULL, TO_WORK); 
	return 0;

__fail:
	esma_msg(me, requester, NULL, RECV_FAILURE);
	return 0;
}

int esma_rx_idle_leave(__unbox__)
{
	return 0;
}

int esma_rx_work_enter(__unbox__)
{
	struct rx_info *rxi = me->data;
	struct esma *req = rxi->requester;
	struct esma_channel *ch = &requester->io_channel;

	if (RX_IO_DISABLE == rxi->status) {
		esma_engine_mod_io_channel(me, ESMA_POLLIN, IO_EVENT_ENABLE);
		rxi->status = RX_IO_ENABLE;
	}

	esma_user_log_dbg("%s()/%s start receiving from fd '%d' for '%s'\n",
			__func__, me->name, ch->fd, req->name);

	return 0;
}

int esma_rx_work_data_0(__unbox__)
{
	struct rx_info *rxi = me->data;
	struct esma_dbuf *dbuf = rxi->dbuf;
	   u32 ba = me->io_channel.info.data.bytes_avail;
	   int n;

	if (0 == ba) {
		esma_user_log_dbg("%s()/%s - connection close\n", __func__, me->name);

		goto __recv_fail;
	}

	if (ba > dbuf->len - dbuf->cnt) {
		int err = esma_dbuf_expand(dbuf, dbuf->cnt + ba);
		if (err) {
			esma_user_log_err("%s()/%s - can't expand dbuf\n", __func__, me->name);
		}
	}

	n = read(me->io_channel.fd, dbuf->pos, ba);
	if (n < 0) {
		if (EINTR == errno)
			return 0;

		goto __recv_fail;
	}

	dbuf->pos += n;
	dbuf->cnt += n;

	if (rxi->read_done(dbuf)) {
		if (rxi->timeout_after_recv) {
			esma_channel_set_interval(rxi->tick_after_recv, rxi->timeout_after_recv);
			esma_msg(me, me, NULL, TO_DONE);
			return 0;
		}

		return _rx_done(me, requester, RECV_SUCCESS);
	}

	return 0;

__recv_fail:

	return _rx_done(me, requester, RECV_FAILURE);
}

int esma_rx_work_data_1(__unbox__)
{
	struct rx_info *rxi = me->data;

	esma_user_log_dbg("%s()/%s - recv: failed\n", __func__, me->name);

	return _rx_done(me, rxi->requester, RECV_FAILURE);	
}

int esma_rx_work_tick_0(__unbox__)
{
	struct rx_info *rxi = me->data;

	esma_user_log_dbg("%s()/%s - recv: timeout\n", __func__, me->name);

	return _rx_done(me, rxi->requester, RECV_FAILURE);	
}

int esma_rx_work_leave(__unbox__)
{
	return 0;
}

int esma_rx_done_enter(__unbox__)
{		
	return 0;
}

int esma_rx_done_tick_0(__unbox__)
{
	struct rx_info *rxi = me->data;

	esma_user_log_dbg("%s()/s - recv for '%s' success\n", __func__, me->name, rxi->requester->name);

	return _rx_done(me, rxi->requester, RECV_SUCCESS);
}

int esma_rx_done_data_0(__unbox__)
{
	esma_msg(me, me, NULL, TO_WORK);
	return 0;
}

int esma_rx_done_data_1(__unbox__)
{
	return esma_rx_work_data_1(requester, me, dptr);;
}

int esma_rx_done_leave(__unbox__)
{
	return 0;
}

int esma_rx_fini_enter(__unbox__)
{
	return 0;
}

int esma_rx_fini_leave(__unbox__)
{
	return 0;
}
