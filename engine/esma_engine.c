
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "core/esma_alloc.h"
#include "core/esma_logger.h"
#include "core/esma_mempool.h"
#include "core/esma_ring_buffer.h"

#include "utils/load_tool.h"

#include "common/compiler.h"

#include "esma_load.h"
#include "esma_engine.h"
#include "esma_message.h"
#include "esma_reactor.h"
#include "esma_engine_info.h"
#include "esma_engine_common.h"
#include "esma_engine_dispatcher.h"

struct reactor *reactor;

static int ngn_cap = 0;
static struct esma_array engines;

#define MQUEUE_LOCK
#define MQUEUE_UNLOCK

#if 0
#define DBG_MSG()		printf("%s() - %d\n", __func__, __LINE__);
#else
#define DBG_MSG()
#endif

int esma_engine_set_number_of_engines(int cap)
{
	int err;

	if (cap < 0)
		exit(1);

	ngn_cap = cap;

	printf("%s() - cap: %d\n", __func__, ngn_cap);
	err = esma_array_init(&engines, ngn_cap, sizeof(struct esma_engine_info));
	if (err) {
		esma_engine_log_ftl("%s() - esma_array_init(%d): failed\n", __func__, cap);
		exit(1);
	}

	return cap;
}

int __esma_set_tick_trans(struct trans *trans, struct esma *esma)
{
	struct esma_channel *ch = &trans->ch;
	   int fd;

	if (0 == ch->type) {
		return 0;
	}

	ch->hard = 1;
	ch->owner = esma;
	ch->info.tick.interval_msec = ch->raw_data;
	fd = reactor->new_timerfd();
	if (unlikely(-1 == fd)) {
		esma_engine_log_err("%s()/%s - reactor->new_timerfd(): failed\n", __func__, esma->name);
		return 1;
	}

	ch->fd = fd;

	reactor->add(fd, ch);
	reactor->mod(fd, ch, 0);
	return 0;
}

int __esma_set_data_trans(struct trans *trans, struct esma *esma)
{
	/* not needed */
	return 0;
}

int __esma_set_sign_trans(struct trans *trans, struct esma *esma)
{
	struct esma_channel *ch = &trans->ch;
	   int fd;

	ch->hard = 1;
	ch->owner = esma;

	fd = reactor->new_sig(ch->raw_data);
	if (unlikely(-1 == fd)) {
		esma_engine_log_err("%s()/%s - reactor->new_sig(): failed\n", __func__, esma->name);
		return 1;
	}

	ch->fd = fd;

	reactor->add(fd, ch);
	reactor->mod(fd, ch, 0);
	return 0;
}

int __esma_register_channels(struct esma *esma)
{
	int err;

	for (int i = 0; i < esma->states.nitems; i++) {
		struct state *state = esma_array_n(&esma->states, i);
		struct trans *trans = NULL;

		for (int j = 0; j < state->tick_trans.nitems; j++) {
			trans = esma_array_n(&state->tick_trans, j);
			err = __esma_set_tick_trans(trans, esma);
			if (err) {
				esma_engine_log_err("%s()/%s - can't set tick trans for '%s state[%d trans]",
						__func__, esma->name, state->name, j);
				goto __fail;
			}
		}

		for (int j = 0; j < state->sign_trans.nitems; j++) {
			trans = esma_array_n(&state->sign_trans, j);
			err = __esma_set_sign_trans(trans, esma);
			if (err) {
				esma_engine_log_err("%s()/%s - can't set sign trans for '%s state[%d trans]",
						__func__, esma->name, state->name, j);
				goto __fail;
			}
		}
	}

	return 0;

__fail:
	return 1;
}

int esma_init(struct esma *esma, char *name, struct esma_template *tmpl, u32 ngn_id)
{
	int err;

	err = esma_load(esma, tmpl);
	if (err) {
		esma_engine_log_ftl("%s() - emsa_load('%s'): failed\n", __func__, tmpl->name);
		return 1;
	}

	if (NULL == name) {
		sprintf(esma->name, "%s", "nameless");
	} else {
		sprintf(esma->name, "%s", name);
	}

	esma->io_channel.type = ESMA_CH_NONE;

	err = __esma_register_channels(esma);
	if (err) {
		esma_engine_log_ftl("%s() - can't register channel\n", __func__, esma->name);
		return 1;
	}

	esma->engine_id = ngn_id;
	return 0;
}

struct esma *esma_new(struct esma_template *tmpl, char *name, u32 ngn_id)
{
	struct esma *esma = NULL;
	   int err;

	if (unlikely(NULL == tmpl)) {
		esma_engine_log_ftl("%s() - tmpl is NULL\n", __func__);
		return NULL;
	}

	esma = esma_malloc(sizeof(struct esma));
	if (unlikely(NULL == esma)) {
		esma_engine_log_ftl("%s() - can't allocate memory for new machine\n", __func__);
		return NULL;
	}

	err = esma_init(esma, name, tmpl, ngn_id);
	if (err) {
		esma_engine_log_ftl("%s()/%s - can't init machine\n", __func__, tmpl->name);
		esma_del(esma);
		esma_free(esma);
		return NULL;
	}

	return esma;
}

void esma_run(struct esma *esma, void *dptr)
{
	struct state *state;

	if (unlikely(NULL == esma)) {
		esma_engine_log_ftl("%s() - esma is NULL\n", __func__);
		exit(1);
	}

	state = esma_array_n(&esma->states, __INIT__);
	if (unlikely(NULL == state)) {
		esma_engine_log_ftl("%s()/%s - init state is NULL\n", __func__, esma->name);
		exit(1);
	}

	esma->current_state = state;

	state->enter(esma, esma, dptr);
}

void esma_del(struct esma *esma)
{
	struct state *fini;

	if (unlikely(NULL == esma))
		return;

	fini = esma_array_n(&esma->states, __FINI__);

	fini->enter(esma, esma, NULL);

	for (int i = 0; i < esma->states.nitems; i++) {
		struct state *state = esma_array_n(&esma->states, i);
		esma_array_free(&state->trans);
	}
	esma_array_free(&esma->states);
}

void esma_msg(struct esma *src, struct esma *dst, void *ptr, u32 code)
{
	struct esma_engine_info *ei;
	struct esma_message *msg;

	ei = esma_array_n(&engines, dst->engine_id);
	msg = esma_ring_buffer_put(&ei->msg_queue);
	if (NULL == msg) {
		esma_engine_log_ftl("%s() - can't send message from '%s' to '%s'\n",
				__func__, src->name, dst->name);
		exit(1);
	}

	msg->src = src;
	msg->dst = dst;
	msg->ptr = ptr;
	msg->code = code;
}

void esma_stop(struct esma *esma)
{
	if (unlikely(NULL == esma))
		return;
}

int esma_engine_init(u32 ngn_id)
{
	struct esma_engine_info *ei;
	int err;

	if (ngn_id > ngn_cap) {
		exit(1);
	}

	ei = esma_array_n(&engines, ngn_id);
	if (NULL == ei) {
		esma_engine_log_ftl("%s() - esma_engine_info[%d] is NULL\n", __func__, ngn_id);
		exit(1);
	}

	if (ei->status) {
		return 1;
	}

	err = esma_ring_buffer_init(&ei->msg_queue, sizeof(struct esma_message), 128);
	if (err) {
		esma_engine_log_ftl("%s() - can't init ring buffer for '%d' engine\n",
				__func__, ngn_id);
		exit(1);
	}

	ei->status = 1;

	if (0 == ngn_id) {
		reactor = get_api("reactor_epoll");	/* For test */
		if (NULL == reactor) {
			esma_engine_log_ftl("%s() - can't get reactor '%s'\n",
					__func__, "reactor_epoll");
			exit(1);
		}
		reactor->init(32, ei);
	}

	return 0;
}

int esma_engine_exec(u32 ngn_id)
{
	struct esma_engine_info *ei;
	struct esma_message *msg;
	   int ret = 0;

	if (unlikely(ngn_id >= ngn_cap)) {
		esma_engine_log_ftl("%s() - invalid engine id: %d\n", __func__, ngn_id);
		exit(1);
	}

	ei = esma_array_n(&engines, ngn_id);
	while (1) {
		msg = esma_ring_buffer_get(&ei->msg_queue);
		if (NULL == msg)
			break;

		ret = esma_engine_dispatcher_send(msg);
	}

	return ret;
}

void esma_engine_wait(void)
{
	reactor->wait();
}

void esma_engine_init_io_channel(struct esma *esma, int fd)
{
	int err;

	if (NULL == esma) {
		esma_engine_log_ftl("%s() - esma is NULL\n", __func__);
		exit(1);
	}

	if (unlikely(fd < 0)) {
		esma_engine_log_ftl("%s()/%s - invalid fd: %d\n", __func__, esma->name, fd);
		exit(1);
	}

	if (ESMA_CH_NONE != esma->io_channel.type)
		return;

	memset(&esma->io_channel, 0, sizeof(struct esma_channel));
	esma->io_channel.type = ESMA_CH_DATA;
	esma->io_channel.hard = 0;
	esma->io_channel.fd = fd;
	esma->io_channel.owner = esma;

	err = reactor->add(fd, &esma->io_channel);
	if (err) {
		esma_engine_log_ftl("%s()/%s - can't add fd '%d' to reactor\n",
				__func__, esma->name, fd);
		exit(1);
	}
}

void esma_engine_free_io_channel(struct esma *esma)
{
	if (NULL == esma)
		exit(1);

	if (esma->io_channel.fd > 0)
		reactor->del(esma->io_channel.fd);

	memset(&esma->io_channel, 0, sizeof(struct esma_channel));
	esma->io_channel.type = ESMA_CH_NONE;
}

void esma_engine_mod_io_channel(struct esma *esma, u32 events, int action)
{
	int err;

	if (unlikely(NULL == esma)) {
		esma_engine_log_ftl("%s() - esma is NULL\n", __func__);
		exit(1);
	}

	if (unlikely(ESMA_CH_NONE == esma->io_channel.type)) {
		esma_engine_log_ftl("%s()/%s - hasen't io channel\n", __func__, esma->name);
		exit(1);
	}

	switch (action) {
		case IO_EVENT_ENABLE:

			err = reactor->mod(esma->io_channel.fd, &esma->io_channel, events);
			if (err) {
				esma_engine_log_ftl("%s()/%s can't enable io channel\n",
						__func__, esma->name);
				exit(1);
			}

			break;

		case IO_EVENT_DISABLE:

			err = reactor->mod(esma->io_channel.fd, &esma->io_channel, 0);
			if (err) {
				esma_engine_log_ftl("%s()/%s cna't disable io channel\n",
						__func__, esma->name);
				exit(1);
			}

			break;

		default:
			exit(1);
	}
}

int esma_engine_mod_channel(struct esma_channel *ch, u32 events)
{
	if (NULL == ch)
		return 0;

	return reactor->mod(ch->fd, ch, events);
}

int esma_engine_arm_tick_channel(struct esma_channel *ch)
{
	if (NULL == ch)
		return 0;

	return reactor->arm_timerfd(ch->fd, ch->info.tick.interval_msec, ch->info.tick.periodic);
}

int esma_engine_disarm_tick_channel(struct esma_channel *ch)
{
	if (NULL == ch)
		return 0;

	return reactor->disarm_timerfd(ch->fd);
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
