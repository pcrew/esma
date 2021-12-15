
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "esma.h"
#include "core/esma_logger.h"
#include "common/compiler.h"
#include "common/macro_magic.h"

static int _read_tick(struct esma_channel *ch)
{
	u64 res;
	int ret;

	ret = read(ch->fd, &res, sizeof(u64));
	if (unlikely(ret != sizeof(u64))) {
		esma_dispatcher_log_err("%s()/%s - filed to read tick\n", __func__, ch->owner->name);
		return 1;
	}

	return 0;
}

static int _read_sign(struct esma_channel *ch)
{
	int ret;
	int n = sizeof(struct signalfd_siginfo);

	ret = read(ch->fd, &ch->info.sign.si, n);
	if (unlikely(n != ret)) {
		esma_dispatcher_log_err("%s()/%s - failed to read sign\n", __func__, ch->owner->name);
		return 1;
	}

	return 0;
}

static int _read_data(struct esma_channel *ch)
{
	int ret;

	if (ch->flags & ESMA_LISTENING_CHANNEL)
		return 0;

	ret = ioctl(ch->fd, FIONREAD, &ch->info.data.bytes_avail);
	if (unlikely(-1 == ret)) {
		esma_dispatcher_log_err("%s()/%s - failed to read data\n", __func__, ch->owner->name);
		return 1;
	}

	return 0;
}

static int _read_fallback(struct esma_channel *ch)
{
	return 0;
}

static int (*_read_ch_data[])(struct esma_channel *ch) = {
	[ESMA_CH_EMPTY] = _read_fallback,
	[ESMA_CH_NONE] = _read_fallback,
	[ESMA_CH_TICK] = _read_tick,
	[ESMA_CH_SIGN] = _read_sign,
	[ESMA_CH_DATA] = _read_data,
};

static int __start_channels(struct state *state)
{
	int err;

	for (int i = 0; i < state->tick_trans.nitems; i++) {
		struct trans *trans = esma_array_n(&state->tick_trans, i);
		struct esma_channel *ch = &trans->ch;

		err = esma_engine_arm_tick_channel(ch);
		if (err) {
			esma_dispatcher_log_ftl("%s()/%s - failed to arm tick channel; id: '%d'\n",
					__func__, ch->owner->name, i);
			exit(1);
		}

		err = esma_engine_mod_channel(ch, ESMA_POLLIN);
		if (err) {
			esma_dispatcher_log_ftl("%s()/%s - failed to mod tick channel; id: '%d'\n",
					__func__, ch->owner->name, i);
			exit(1);
		}
	}

	for (int i = 0; i < state->sign_trans.nitems; i++) {
		struct trans *trans = esma_array_n(&state->sign_trans, i);
		struct esma_channel *ch = &trans->ch;

		err = esma_engine_mod_channel(ch, ESMA_POLLIN);
		if (err) {
			esma_dispatcher_log_ftl("%s()/%s - failed to mod sign channel; id: '%d'\n",
					__func__, ch->owner->name, i);
			exit(1);
		}
	}

	return 0;
}

static int __stop_channels(struct state *state)
{
	int err;

	for (int i = 0; i < state->tick_trans.nitems; i++) {
		struct trans *trans = esma_array_n(&state->tick_trans, i);
		struct esma_channel *ch = &trans->ch;

		err = esma_engine_disarm_tick_channel(ch);
		if (err) {
			esma_dispatcher_log_bug("%s()/%s - failed to mod tick channel; fd: '%d'\n",
					__func__, ch->owner->name, i);
			return 1;
		}
	}

	for (int i = 0; i < state->sign_trans.nitems; i++) {
		struct trans *trans = esma_array_n(&state->tick_trans, i);
		struct esma_channel *ch = &trans->ch;

		err = esma_engine_mod_channel(ch, 0);
		if (err) {
			esma_dispatcher_log_bug("%s()/%s - failed to mod sign channel; fd: '%d'\n",
					__func__, ch->owner->name, i);
			return 1;
		}
	}

	return 0;
}

int esma_engine_dispatcher_send(struct esma_message *msg)
{
	struct esma_channel *ch;
	struct state *state;
	struct trans *trans;

	struct esma *src = msg->src;
	struct esma *dst = msg->dst;
	  void      *ptr = msg->ptr;
	   u32      code = msg->code;

	   int err;

	if (NULL == src && NULL == dst) { /* message from os */
		goto __read_os_msg;
	}

	state = dst->current_state;
	if (unlikely(NULL == state)) {
		esma_dispatcher_log_bug("%s()/%s - current state is NULL\n", __func__, dst->name);
		goto __fail;
	}

	trans = esma_array_n(&state->trans, code);
	if (NULL == trans) {
		esma_dispatcher_log_bug("%s()/%s - trans[%d] is NULL\n", __func__, dst->name, code);
		goto __fail;
	}

	goto __send_msg;

__read_os_msg:

	ch = ptr;

	src = ch->owner;
	dst = ch->owner;
	ptr = NULL;

	state = dst->current_state;
	if (ch->hard) {
		trans = container_of(ch, struct trans, ch);
	} else { /* is IO channel */
		switch (code) {
		case ESMA_POLLIN:
			trans = &state->data_pollin;
			break;

		case ESMA_POLLOUT:
			trans = &state->data_pollout;
			break;

		case ESMA_POLLERR:
		case ESMA_POLLHUP:
			trans = &state->data_pollerr;
			break;

		default:
			esma_dispatcher_log_bug("%s()/%s - invalid IO channel\n", __func__, dst->name);
			goto __fail;
		}
	}

	err = _read_ch_data[ch->type](ch);
	if (err) {
		esma_dispatcher_log_bug("%s()/%s - can't read channel data\n", __func__, dst->name);
		goto __fail;
	}

	goto __send_msg;

__send_msg:

	/* Meely section enter */
	if (NULL == trans->next_state) {
		esma_dispatcher_log_dbg("Esma '%s' ACCEPTED message from '%s' with code %d: self\n",
				dst->name, src->name, code);
		return trans->action(src, dst, ptr);
	}
	/* Meely section leave */

	/* Moore section enter */
	err = __stop_channels(state);
	if (err) {
		esma_dispatcher_log_ftl("%s()/%s - __stop_channels('%s'): failed \n",
				__func__, dst->name, state->name);
		goto __fail;
	}

	err = state->leave(src, dst, ptr);
	if (err) {
		goto __fail;
	}

	dst->current_state = trans->next_state;
	state = dst->current_state;

	esma_dispatcher_log_dbg("Esma '%s' ACCEPTED message from '%s' with code %d; next state: %s\n",
			dst->name, src->name, code, state->name);

	err = state->enter(src, dst, ptr);
	if (err) {
		return state->leave(src, dst, ptr);
	}

	err = __start_channels(state);
	if (err) {
		esma_dispatcher_log_ftl("%s()/%s - __start_channels('%s'): failed\n",
				__func__, dst->name, state->name);
		goto __fail;
	}
	/* Moore section leave */

	return 0;

__fail:
	return 1;
}
