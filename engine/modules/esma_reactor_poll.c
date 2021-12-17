
#include <unistd.h>
#include <errno.h>
#include <poll.h>

#include "common/compiler.h"

#include "core/esma_fd.h"
#include "core/esma_array.h"
#include "core/esma_alloc.h"
#include "core/esma_logger.h"
#include "core/esma_mempool.h"
#include "core/esma_ring_buffer.h"

#include "engine/esma.h"

static struct esma_engine_info *ei = NULL;

struct esma_array event_list = {0};
struct esma_array channels = {0};

static void poll_reactor__init(u32 nev, void *tools)
{
	int err;

	if (NULL != ei) {
		esma_reactor_log_ftl("%s() - double reactor initialization\n", __func__);
		exit(0);
	}

	ei = tools;

	err = esma_array_init(&event_list, nev, sizeof(struct pollfd));
	if (err) {
		esma_reactor_log_ftl("%s() - can't init events list\n", __func__);
		exit(1);
	}

	err = esma_array_init(&channels, nev, sizeof(struct esma_channels *));
	if (err) {
		esma_reactor_log_ftl("%s() - can't init channels list\n", __func__);
		exit(1);
	}
}

static void poll_reactor__fini(void)
{
	esma_array_free(&event_list);
	esma_array_free(&channels);
}

static int poll_reactor__add(int fd, struct esma_channel *ch)
{
	struct pollfd *event = esma_array_push(&event_list);
	struct esma_channel **channel = esma_array_push(&channels);

	event->fd = fd;
	*channel = ch;
	
	ch->index = event_list.nitems - 1;
	return 0;
}

static int poll_reactor__del(int fd, struct esma_channel *ch)
{
	if (unlikely(ch->index < 0))
		return 1;

	struct pollfd *event = event_list.items;
	struct esma_channel **channel = channels.items;

	event[ch->index] = event[event_list.nitems - 1];
	channel[ch->index] = channel[channels.nitems - 1];
	channel[ch->index]->index = ch->index;
	ch->index = -1;

	event[--event_list.nitems] = (struct pollfd) {0};
	channel[--channels.nitems] = NULL;

	return 0;
}

static int poll_reactor__mod(int fd, struct esma_channel *ch, u32 event)
{
	struct pollfd *ev = esma_array_n(&event_list, ch->index);
	u32 e = 0;

	if (unlikely(ch->index < 0))
		return 1;

	if (event & ESMA_POLLIN) {
		e |= POLLIN;
	}

	if (event & ESMA_POLLOUT) {
		e |= POLLOUT;
	}

	ev->events = e;
	return 0;
}

static void poll_reactor__wait(void)
{
	struct esma_ring_buffer *msg_queue = &ei->msg_queue;
	int ready;

	ready = poll(event_list.items, event_list.nitems, -1);

	if (unlikely(-1 == ready)) {
		if (unlikely(EINTR == errno))
			return;

		esma_reactor_log_sys("%s() - poll(nitems: %d) failed: %s\n", __func__, event_list.nitems, strerror(errno));
		exit(1);
	}

	for (int i = 0; i < event_list.nitems && ready; i++) {
		struct pollfd *pfd = esma_array_n(&event_list, i);
		struct esma_channel **ch = esma_array_n(&channels, i);
		struct esma_message *msg;
		u32 revents;
		u32 e = 0;

		if (-1 == pfd->fd || 0 == pfd->revents)
			continue;

		revents = pfd->revents;

		if (revents & POLLIN)
			e |= ESMA_POLLIN;
		else if (revents & POLLOUT)
			e |= ESMA_POLLOUT;
		else if (revents & POLLERR)
			e |= ESMA_POLLERR;
		else if (revents & POLLHUP)
			e |= ESMA_POLLHUP;	

		msg = esma_ring_buffer_put(msg_queue);
		if (unlikely(NULL == msg)) {
			esma_reactor_log_ftl("%s() - can't put msg\n", __func__);
			exit(1);
		}

		msg->src = NULL;
		msg->dst = NULL;
		msg->ptr = *ch;
		msg->code = e;

		ready--;
	}
}

api_definition(reactor, reactor_poll) {
	.init = poll_reactor__init,
	.fini = poll_reactor__fini,

	.add = poll_reactor__add,
	.del = poll_reactor__del,
	.mod = poll_reactor__mod,

	.wait = poll_reactor__wait,
};
