
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "esma_rx.h"
#include "esma_sm_data.h"

#include "core/esma_dbuf.h"
#include "core/esma_logger.h"

char *esma_rx_tmpl = 
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
	"		work -> done: 0;				"
	"								"
	"		done -> self: tick_0: 0ms: ESMA_TM_ONESHOT	"
	"		done -> self: data_0: ESMA_POLLIN;		"
	"		done -> self: data_1: ESMA_POLLERR;		"
	"		done -> idle: 0;				"
	"		done -> work: 1;				"	
	"	};							"
	"								"
;

/* Public functions */
int esma_rx_init(struct esma *rx, char *name, char *tmpl_path, int ngn_id)
{
	struct esma_dbuf esma_rx_tmpl_dbuf;
	struct esma_template tmpl;
	   int err;

	esma_dbuf_set(&esma_rx_tmpl_dbuf, esma_rx_tmpl);

	if (NULL == name)
		name = "nameless";

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

	err = esma_template_set_by_dbuf(&tmpl, &esma_rx_tmpl_dbuf);
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

	err = esma_init(rx, name, &tmpl. ngn_id);
	if (err) {
		esma_user_log_err("%s()/%s: esma_init(): failed\n",
				__func__, name);
		return 1;
	}

	esma_user_log_dbg("%s()/%s - machine successfuly inited\n", __func__, name);
	return 0;	
}

int esma_rx_run(struct esma *rx, struct esma_objpool *restroom)
{
	if (NULL == rx) {
		esma_user_log_err("%s() - rx or restroom is NULL\n", __func__);
		return 1;
	}

	return esma_run(&master, restroom);
}

/* State machine actions */

#define __unbox__	struct esma *requester, struct esma *me, void *dptr

#define RX_STATUS_EMPTY		0
#define RX_STATUS_READING	1
#define RX_STATUS_CHECKING	2
#define RX_STATUS

struct rx_info {
	struct esma *requester;
	struct esma_dbuf *dbuf;
	struct esma_objpool *restroom;
	struct esma_channel *tick_waiting;
	struct esma_channel *tick_after_recv;
	u32 timeout_after_read;

	read_done_f read_done;
	u32 status;
};

int esma_rx_init_enter(__unbox__)
{
	struct rx_info *rxi = NULL;
	
	rxi = malloc(sizeof(struct rx_info));
	if (NULL == rxi) {
		esma_user_log_err("%s()/%s - can't allocate memory for info section\n",
				__func__, me->name);
		goto __fini;
	}

	rxi->restroom = dptr;
	rxi->tick_waiting = esma_get_channel(me, "work", 0, ESMA_CH_TICK);
	if (NULL == rxi->tick_waiting) {
		esma_user_log_wrn("%s()/%s - can't get channel 0 for 'work' state. Did you add tick_0 in esma file?\n",
				__func__, me->name);
	}

	rxi->tick_after_recv = esma_get_channel(me, "done", 0, ESMA_CH_TICK);
	if (NULL == rxi->tick_after_recv) {
		esma_user_log_wrn("%s()/%s - can't get channel 0 for 'done' state. Did you add tick_0 in esma file?\n",
				__func__, me->name);
	}

	me->data = rxi;
	esma_msg(me, me, NULL, 0);
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

	err = esma_objpool_put(pool, me);
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
				__func__, me->name, from->name);

		esma_msg(me, requester, NULL, 2);
		return 0;
	}

	rxi->requester = from;
	rxi->dbuf = ctx->dbuf;
	rxi->timeout_after_recv = ctx->timeout_after_recv;
	rxi->read_done = ctx->read_done;

	esma_channel_set_interval(rxi->tick_waiting, mtr->timeout_wait);

	esma_msg(me, me, NULL, 0); 
	return 0;
}

int esma_rx_idle_leave(__unbox__)
{
	return 0;
}

int esma_rx_work_enter(__unbox__)
{
	struct rx_info *rxi = me->data;
	struct esma *requester = rxi->requester;
	struct esma_channel *ch = &requester->io_channel;

	esma_dbuf_clear(rxi->dbuf);

	esma_engine_mod_io_channel(ch, ESMA_POLLIN, IO_EVENT_ENABLE);
	esma_user_log_dbg("%s()/%s start receiving from fd '%d' for '%s'\n",
			__func__, me->name, ch->fd, requester->name);

	return 0;

}

int esma_rx_work_data_0(__unbox__)
{
	struct rx_info *rxi = me->data;
	struct esma_dbuf *dbuf = rxi->dbuf;
	   u32 ba = esma_channel_bytes_avail(&rxi->requester->io_channel);
	   int n;

	if (0 == ba) {
		esma_user_log_dbg("%s()/%s - connection close\n", __func__, me->name);
		esma_msg(me, requester, NULL, 2);
		esma_msg(me, me, NULL, 0);
		return 0;
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

		esma_msg(me, requester, NULL, 2);
		esma_msg(me, me, NULL, 0);
		return 0;
	}

	dbuf->pos += n;
	dbuf->cnt += n;

	if (rxi->read_done(dbuf)) {
		if (rxi->timeout_after_read) {
			esma_channel_set_interval(rxi->tick_after_recv, rxi->timeout_after_recv);
			esma_msg(me, me, NULL, 0);
			return 0;
		}

		esma_msg(me, requester, NULL, 1);
		esma_msg(me, me, NULL, 0);
		return 0;
	}

	return 0;
}

int esma_rx_work_tick_0(__unbox__)
{
	struct esma_rx_info *rxi = me->data;

	esma_user_log_dbg("%s()/%s - recv: timeout\n", __func__, me->name);
	esma_msg(me, rxi->requester, NULL, 2);
	esma_msg(me, me, NULL, 0);
	return 0;
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
	return 0;
}

int esma_rx_done_data_0(__unbox__)
{
	return 0;
}

int esma_rx_done_data_1(__unbox__)
{
	return 0;
}

int esma_rx_done_leave(__unbox__)
{
	return 0;
}
