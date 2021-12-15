
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

static struct reactor *reactor;
static struct esma_engine_info engine_info;

static int _esma_set_tick_trans(struct trans *trans, struct esma *esma)
{
	struct esma_channel *ch = &trans->ch;
	   int fd;

	if (0 == ch->type) {
		return 0;
	}

	ch->hard = 1;
	ch->owner = esma;
	ch->info.tick.interval_msec = ch->raw_data;

	fd = esma_engine_new_timerfd();
	if (unlikely(-1 == fd)) {
		esma_engine_log_err("%s()/%s - reactor->new_timerfd(): failed('%s')\n", __func__, esma->name, strerror(errno));
		return 1;
	}

	ch->fd = fd;

	reactor->add(fd, ch);
	reactor->mod(fd, ch, 0);
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
		esma_engine_log_err("%s()/%s - reactor->new_sig(): failed('%s')\n", __func__, esma->name, strerror(errno));
		return 1;
	}

	ch->fd = fd;

	reactor->add(fd, ch);
	reactor->mod(fd, ch, 0);
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
				esma_engine_log_ftl("%s()/%s - can't set tick trans for '%s state[%d trans]",
						__func__, esma->name, state->name, j);
				goto __fail;
			}
		}

		for (int j = 0; j < state->sign_trans.nitems; j++) {
			trans = esma_array_n(&state->sign_trans, j);
			err = _esma_set_sign_trans(trans, esma);
			if (err) {
				esma_engine_log_ftl("%s()/%s - can't set sign trans for '%s state[%d trans]",
						__func__, esma->name, state->name, j);
				goto __fail;
			}
		}
	}

	return 0;

__fail:
	return 1;
}

int esma_engine_init_machine(struct esma *esma, char *name, struct esma_template *tmpl)
{
	int err;

	err = esma_load(esma, tmpl);
	if (err) {
		esma_engine_log_ftl("%s() - emsa_load('%s'): failed('%s')\n", __func__, tmpl->name, strerror(errno));
		return 1;
	}

	if (NULL == name) {
		sprintf(esma->name, "%s", "nameless");
	} else {
		sprintf(esma->name, "%s", name);
	}

	esma->io_channel.type = ESMA_CH_NONE;

	err = _esma_register_channels(esma);
	if (err) {
		esma_engine_log_ftl("%s() - can't register channel\n", __func__, esma->name, strerror(errno));
		return 1;
	}

	return 0;
}

struct esma *esma_engine_new_machine(struct esma_template *tmpl, char *name)
{
	struct esma *esma = NULL;
	   int err;

	if (unlikely(NULL == tmpl)) {
		esma_engine_log_err("%s() - tmpl is NULL\n", __func__);
		return NULL;
	}

	esma = esma_malloc(sizeof(struct esma));
	if (unlikely(NULL == esma)) {
		esma_engine_log_err("%s() - can't allocate memory for new machine\n", __func__);
		return NULL;
	}

	err = esma_engine_init_machine(esma, name, tmpl);
	if (err) {
		esma_engine_log_err("%s()/%s - can't init machine\n", __func__, tmpl->name);
		esma_engine_del_machine(esma);
		esma_free(esma);
		return NULL;
	}

	return esma;
}

int esma_engine_run_machine(struct esma *esma, void *dptr)
{
	struct state *state;

	if (unlikely(NULL == esma)) {
		esma_engine_log_err("%s() - esma is NULL\n", __func__);
		return 1;
	}

	state = esma_array_n(&esma->states, __INIT__);
	if (unlikely(NULL == state)) {
		esma_engine_log_err("%s()/%s - init state is NULL\n", __func__, esma->name);
		return 1;
	}

	esma->current_state = state;

	return state->enter(esma, esma, dptr);
}

int esma_engine_copy_machine(struct esma *src, struct esma *dst, int (*copy_f)(const void *data_src, void *data_dst))
{
	if (NULL == src || NULL == dst) {
		esma_engine_log_dbg("%s() - src or dst esma is NULL.\n", __func__);
		return 1;
	}

	memcpy(src, dst, sizeof(struct esma));
	esma_array_copy(&src->states, &dst->states);
	for (int i = 0; i < dst->states.nitems; i++) {
		struct state *state_src = esma_array_n(&src->states, i);
		struct state *state_dst = esma_array_n(&dst->states, i);

		esma_array_copy(&state_src->trans, &state_dst->trans);
		esma_array_copy(&state_src->sign_trans, &state_dst->sign_trans);
		esma_array_copy(&state_src->tick_trans, &state_dst->tick_trans);
	}

	copy_f(src->data, dst->data);
	return 0;
}

int esma_engine_restart_machine(struct esma *esma)
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

int esma_engine_del_machine(struct esma *esma)
{
	struct state *states;
	if (unlikely(NULL == esma)) {
		esma_engine_log_err("%s() - esma is NULL.\n", __func__);
		return 1;
	}

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
	esma_array_free(&esma->states);
	return 0;
}

void esma_engine_send_msg(struct esma *src, struct esma *dst, void *ptr, u32 code)
{
	struct esma_message *msg;

	msg = esma_ring_buffer_put(&engine_info.msg_queue);
	if (NULL == msg) {
		esma_engine_log_err("%s() - can't send message from '%s' to '%s'\n",
				__func__, src->name, dst->name);
		exit(1);
	}

	msg->src = src;
	msg->dst = dst;
	msg->ptr = ptr;
	msg->code = code;
}

int esma_engine_init(char *reactor_name)
{
	int err;

	if (NULL == reactor_name) {
		esma_engine_log_inf("%s() - nameless reactor; using 'reactor_epll'\n", __func__);
		reactor_name = "reactor_epoll";
	}

	if (engine_info.status) {
		return 1;
	}

	err = esma_ring_buffer_init(&engine_info.msg_queue, sizeof(struct esma_message), 128);
	if (err) {
		esma_engine_log_ftl("%s() - can't init ring buffer for engine\n",
				__func__);
		exit(1);
	}

	engine_info.status = 1;

	reactor = get_api(reactor_name);
	if (NULL == reactor) {
		esma_engine_log_ftl("%s() - can't get reactor '%s'\n",
				__func__, reactor_name);
		exit(1);
	}
	reactor->init(32, &engine_info);

	return 0;
}

int esma_engine_exec()
{
	struct esma_message *msg;
	   int ret = 0;

	esma_time_update();

	while (1) {
		msg = esma_ring_buffer_get(&engine_info.msg_queue);
		if (NULL == msg)
			break;

		ret = esma_engine_dispatcher_send(msg);
		if (ret)
			break;
	}

	return ret;
}

void esma_engine_wait(void)
{
	reactor->wait();
}

int esma_engine_init_io_channel(struct esma *esma, int fd)
{
	int err;

	if (NULL == esma) {
		esma_engine_log_ftl("%s() - esma is NULL\n", __func__);
		return 1;
	}

	if (unlikely(fd < 0)) {
		esma_engine_log_ftl("%s()/%s - invalid fd: %d\n", __func__, esma->name, fd);
		return 1;
	}

	if (ESMA_CH_NONE != esma->io_channel.type) {
		esma_engine_log_dbg("%s()/%s - esma hasn't io channel\n", __func__, esma->name);
		return 0;
	}

	memset(&esma->io_channel, 0, sizeof(struct esma_channel));
	esma->io_channel.type = ESMA_CH_DATA;
	esma->io_channel.hard = 0;
	esma->io_channel.fd = fd;
	esma->io_channel.owner = esma;

	err = reactor->add(fd, &esma->io_channel);
	if (err) {
		esma_engine_log_ftl("%s()/%s - can't add fd '%d' to reactor\n",
				__func__, esma->name, fd);
		return 1;
	}

	return 0;
}

int esma_engine_free_io_channel(struct esma *esma)
{
	if (unlikely(NULL == esma)) {
		esma_engine_log_dbg("%s() - esma is NULL.\n", __func__);
		return 0;
	}

	if (esma->io_channel.fd > 0) {
		reactor->del(esma->io_channel.fd, &esma->io_channel);
	}

	memset(&esma->io_channel, 0, sizeof(struct esma_channel));
	esma->io_channel.type = ESMA_CH_NONE;
	return 0;
}

int esma_engine_mod_io_channel(struct esma *esma, u32 events, int action)
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
		err = reactor->mod(esma->io_channel.fd, &esma->io_channel, events);
		if (err) {
			esma_engine_log_ftl("%s()/%s - can't enable io channel\n",
					__func__, esma->name);
			return 1;
		}
		break;

	case ESMA_IO_EVENT_DISABLE:
		err = reactor->mod(esma->io_channel.fd, &esma->io_channel, 0);
		if (err) {
			esma_engine_log_ftl("%s()/%s - can't disable io channel\n",
					__func__, esma->name);
			return 1;
		}
		break;

	default:
		esma_engine_log_dbg("%s()/%s - invalid action\n", __func__, esma->name);
	}

	return 0;
}

int esma_engine_mod_channel(struct esma_channel *ch, u32 events)
{
	if (NULL == ch)
		return 1;

	return reactor->mod(ch->fd, ch, events);
}

int esma_engine_arm_tick_channel(struct esma_channel *ch)
{
	if (NULL == ch)
		return 1;

	return esma_engine_arm_timerfd(ch->fd, ch->info.tick.interval_msec, ch->info.tick.periodic);
}

int esma_engine_disarm_tick_channel(struct esma_channel *ch)
{
	if (NULL == ch)
		return 1;

	return esma_engine_disarm_timerfd(ch->fd);
}

struct state *esma_find_state_by_name(struct esma *esma, char *name)
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

struct esma_channel *esma_get_channel(struct esma *esma, char *state_name, int id, u32 type)
{
	struct state *state = NULL;
	struct trans *trans = NULL;

	if (unlikely(NULL == esma || id < 0))
		return NULL;

	state = esma_find_state_by_name(esma, state_name);
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
		break;
	}

	if (NULL == trans)
		return NULL;

	return &trans->ch;
}
