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

static int _esma_set_tick_trans(struct trans *trans, struct esma *esma)
{
	struct esma_channel *ch = &trans->ch;
	   int fd;

	ch->hard = 1;
	ch->owner = esma;
	ch->info.tick.interval_msec = ch->raw_data;

	fd = esma_engine_new_timerfd();
	if (unlikely(-1 == fd)) {
		esma_engine_log_err("%s()/%s - reactor->new_timerfd() failed.\n", __func__, esma->name);
		return 1;
	}

	ch->fd = fd;

	esma->engine->reactor.ops.add(&esma->engine->reactor.reactor, fd, ch, 0);
	return 0;
}

static int _esma_set_sign_trans(struct trans *trans, struct esma *esma)
{
	struct esma_channel *ch = &trans->ch;
	   int fd;

	ch->hard = 1;
	ch->owner = esma;

	fd = esma_engine_new_signalfd(ch->raw_data);
	if (unlikely(-1 == fd)) {
		esma_engine_log_err("%s()/%s - reactor->new_sig() failed.\n", __func__, esma->name);
		return 1;
	}

	ch->fd = fd;

	esma->engine->reactor.ops.add(&esma->engine->reactor.reactor, fd, ch, 0);
	return 0;
}

static int _esma_register_channels(struct esma *esma)
{
	int err;

	for (int i = 0; i < esma->states.nitems; i++) {
		struct state *state = esma_array_n(&esma->states, i);
		struct trans *trans = NULL;

		for (int j = 0; j < state->tick_trans.nitems; j++) {
			trans = esma_array_n(&state->tick_trans, j);
			err = _esma_set_tick_trans(trans, esma);
			if (err) {
				esma_engine_log_ftl("%s()/%s - can't set tick trans for '%s state[%d trans].",
						__func__, esma->name, state->name, j);
				goto __fail;
			}
		}

		for (int j = 0; j < state->sign_trans.nitems; j++) {
			trans = esma_array_n(&state->sign_trans, j);
			err = _esma_set_sign_trans(trans, esma);
			if (err) {
				esma_engine_log_ftl("%s()/%s - can't set sign trans for '%s state[%d trans].",
						__func__, esma->name, state->name, j);
				goto __fail;
			}
		}
	}

	return 0;

__fail:
	return 1;
}

static int _esma_machine_init(struct esma *esma, struct esma_engine *ngn, struct esma_template *tmpl, char *name)
{
	int err;

	err = esma_load(esma, tmpl);
	if (err) {
		esma_engine_log_ftl("%s() - emsa_load('%s') failed.\n", __func__, tmpl->name);
		return 1;
	}

	sprintf(esma->name, "%s", name ? name : "nameless");

	esma->io_channel.type = ESMA_CH_NONE;

	esma->engine = ngn;
	err = _esma_register_channels(esma);
	if (err) {
		esma_engine_log_ftl("%s/%s() - can't register channel.\n", __func__, esma->name);
		return 1;
	}

	return 0;
}

struct esma *esma_machine_new(struct esma_engine *ngn, struct esma_template *tmpl, char *name)
{
	struct esma *esma;
	   int err;

	if (unlikely(NULL == tmpl)) {
		esma_engine_log_err("%s() - tmpl is NULL.\n", __func__);
		return NULL;
	}

	esma = esma_mempool_get_block(&ngn->machine_pool);
	if (unlikely(NULL == esma)) {
		esma_engine_log_err("%s() - can't get memory for new machine.\n", __func__);
		return NULL;
	}

	err = _esma_machine_init(esma, ngn, tmpl, name);
	if (err) {
		esma_engine_log_err("%s()/%s - can't init machine.\n", __func__, tmpl->name);
		esma_machine_del(esma);
		return NULL;
	}

	return esma;
}

int esma_machine_run(struct esma *esma, void *dptr)
{
	struct state *state;

	if (unlikely(NULL == esma)) {
		esma_engine_log_err("%s() - esma is NULL.\n", __func__);
		return 1;
	}

	state = esma_array_n(&esma->states, __INIT__);
	if (unlikely(NULL == state)) {
		esma_engine_log_err("%s()/%s - init state is NULL.\n", __func__, esma->name);
		return 1;
	}

	esma->current_state = state;

	return state->enter(esma, esma, dptr);
}

int esma_machine_restart(struct esma *esma)
{
	struct state *states;
	if (NULL == esma) {
		esma_engine_log_dbg("%s() - esma is NULL.\n", __func__);
		return 1;
	}

	states = esma->states.items;

	if (esma->current_state != &states[__FINI__]) {
		esma_engine_log_dbg("%s() - esma in work.\n", __func__);
		return 1;
	}

	esma->current_state = &states[__INIT__];
	return 0;
}

int esma_machine_del(struct esma *esma)
{
	struct esma_engine *ngn;
	struct state *states;
	int err;

	if (unlikely(NULL == esma)) {
		esma_engine_log_err("%s() - esma is NULL.\n", __func__);
		return 1;
	}

	ngn = esma->engine;
	states = esma->states.items;

	if (esma->current_state != &states[__FINI__]) {
		esma_engine_log_dbg("%s() - esma in work.\n", __func__);
		return 1;
	}

	for (int i = 0; i < esma->states.nitems; i++) {
		struct state *state = esma_array_n(&esma->states, i);
		esma_array_free(&state->trans);
		esma_array_free(&state->sign_trans);
		esma_array_free(&state->tick_trans);
	}

	err = esma_mempool_put_block(&ngn->machine_pool, esma);
	if (unlikely(err)) {
		esma_engine_log_err("%s() - can't put esma in the machine pool.\n", __func__);
		return 1;
	}

	return 0;
}

int esma_machine_init_io_channel(struct esma *esma, int fd)
{
	int err;

	if (NULL == esma) {
		esma_engine_log_ftl("%s() - esma is NULL.\n", __func__);
		return 1;
	}

	if (unlikely(fd < 0)) {
		esma_engine_log_ftl("%s()/%s - invalid fd: %d.\n", __func__, esma->name, fd);
		return 1;
	}

	if (ESMA_CH_NONE != esma->io_channel.type) {
		esma_engine_log_dbg("%s()/%s - esma hasn't io channel.\n", __func__, esma->name);
		return 0;
	}

	memset(&esma->io_channel, 0, sizeof(struct esma_channel));
	esma->io_channel.type = ESMA_CH_DATA;
	esma->io_channel.hard = 0;
	esma->io_channel.fd = fd;
	esma->io_channel.owner = esma;

	err = esma->engine->reactor.ops.add(&esma->engine->reactor.reactor, fd, &esma->io_channel, 0);
	if (err) {
		esma_engine_log_bug("%s()/%s - can't add fd '%d' to reactor\n",
				__func__, esma->name, fd);
		return 1;
	}

	return 0;
}

int esma_machine_free_io_channel(struct esma *esma)
{
	if (unlikely(NULL == esma)) {
		esma_engine_log_dbg("%s() - esma is NULL.\n", __func__);
		return 0;
	}

	if (esma->io_channel.fd > 0) {
		esma->engine->reactor.ops.del(&esma->engine->reactor.reactor, esma->io_channel.fd, &esma->io_channel);
	}

	memset(&esma->io_channel, 0, sizeof(struct esma_channel));
	esma->io_channel.type = ESMA_CH_NONE;
	return 0;
}

int esma_machine_mod_io_channel(struct esma *esma, u32 events, int action)
{
	int err;

	if (unlikely(NULL == esma)) {
		esma_engine_log_dbg("%s() - esma is NULL\n", __func__);
		return 0;
	}

	if (unlikely(ESMA_CH_NONE == esma->io_channel.type)) {
		esma_engine_log_dbg("%s()/%s - hasn't io channel\n", __func__, esma->name);
		return 0;
	}

	switch (action) {
	case ESMA_IO_EVENT_ENABLE:
		err = esma->engine->reactor.ops.mod(&esma->engine->reactor.reactor, esma->io_channel.fd, &esma->io_channel, events);
		if (err) {
			esma_engine_log_bug("%s()/%s - can't enable io channel\n",
					__func__, esma->name);
			return 1;
		}
		break;

	case ESMA_IO_EVENT_DISABLE:
		err = esma->engine->reactor.ops.mod(&esma->engine->reactor.reactor, esma->io_channel.fd, &esma->io_channel, 0);
		if (err) {
			esma_engine_log_bug("%s()/%s - can't disable io channel\n",
					__func__, esma->name);
			return 1;
		}
		break;

	default:
		esma_engine_log_bug("%s()/%s - invalid action\n", __func__, esma->name);
		return 1;
	}

	return 0;
}

int esma_machine_mod_channel(struct esma *esma, struct esma_channel *ch, u32 events)
{
	if (NULL == ch)
		return 1;

	return esma->engine->reactor.ops.mod(&esma->engine->reactor.reactor, ch->fd, ch, events);
}

int esma_machine_arm_tick_channel(struct esma_channel *ch)
{
	if (NULL == ch)
		return 1;

	return esma_engine_arm_timerfd(ch->fd, ch->info.tick.interval_msec, ch->info.tick.periodic);
}

int esma_machine_disarm_tick_channel(struct esma_channel *ch)
{
	if (NULL == ch)
		return 1;

	return esma_engine_disarm_timerfd(ch->fd);
}

struct state *esma_machine_find_state_by_name(struct esma *esma, char *name)
{
	if (unlikely(NULL == esma || NULL == name)) {
		return NULL;
	}

	for (int i = 0; i < esma->states.nitems; i++) {
		struct state *state = esma_array_n(&esma->states, i);

		if (strcmp(state->name, name))
			continue;

		return state;
	}

	return NULL;
}

struct esma_channel *esma_machine_get_channel(struct esma *esma, char *state_name, int id, u32 type)
{
	struct state *state = NULL;
	struct trans *trans = NULL;

	if (unlikely(NULL == esma || id < 0))
		return NULL;

	state = esma_machine_find_state_by_name(esma, state_name);
	if (NULL == state)
		return NULL;

	switch (type) {
	case ESMA_CH_TICK:
		trans = esma_array_n(&state->tick_trans, id);
		break;

	case ESMA_CH_SIGN:
		trans = esma_array_n(&state->sign_trans, id);
		break;

	default:
		goto __channel_not_found;
	}

	return &trans->ch;

__channel_not_found:
	esma_engine_log_dbg("%s()/%s - can't get channel '%s'\n", __func__, esma->name, esma_channel_type_to_str(type));
	return NULL;
}
