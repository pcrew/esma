
#include <unistd.h>
#include <errno.h>
#include <poll.h>

#include "common/compiler.h"
#include "common/macro_magic.h"

#include "core/esma_fd.h"
#include "core/esma_array.h"
#include "core/esma_alloc.h"
#include "core/esma_logger.h"
#include "core/esma_mempool.h"
#include "core/esma_ring_buffer.h"

#include "engine/esma.h"

static int poll_reactor__init(union reactor *reactor, u32 nevents)
{
	int err = esma_array_init(&reactor->poll.event_list, nevents, sizeof(struct pollfd));
	if (err) {
		esma_reactor_log_ftl("%s() - can't init list for %d events.\n", __func__, nevents);
		return err;
	}

	err = esma_array_init(&reactor->poll.channels, nevents, sizeof(struct esma_channels *));
	if (err) {
		esma_reactor_log_ftl("%s() - can't init list for %d channels.\n", __func__, nevents);
		return err;
	}

	return 0;
}

static void poll_reactor__fini(union reactor *reactor)
{
	esma_array_free(&reactor->poll.event_list);
	esma_array_free(&reactor->poll.channels);
}

ESMA_INLINE static void __mod__(struct pollfd *event, u32 events)
{
	u32 ev = 0;

	if (events & ESMA_POLLIN)
		ev |= POLLIN;
	if (events & ESMA_POLLOUT)
		ev |= POLLOUT;

	event->events = ev;
}

static int poll_reactor__add(union reactor *reactor, int fd, struct esma_channel *ch, u32 events)
{
	struct pollfd *event = esma_array_push(&reactor->poll.event_list);
	struct esma_channel **channel = esma_array_push(&reactor->poll.channels);

	event->fd = fd;
	*channel = ch;

	if (events)
		__mod__(event, events);
	
	ch->index = reactor->poll.event_list.nitems - 1;
	return 0;
}

static int poll_reactor__del(union reactor *reactor, int fd, struct esma_channel *ch)
{
	struct pollfd last_event;
	struct esma_channel *last_channel;

	if (unlikely(ch->index < 0))
		return 1;

	esma_array_pop(&reactor->poll.event_list, &last_event);
	esma_array_pop(&reactor->poll.channels, &last_channel);

	((struct pollfd *) reactor->poll.event_list.items)[ch->index] = last_event;
	((struct esma_channel **) reactor->poll.channels.items)[ch->index] = last_channel;
	((struct esma_channel **) reactor->poll.channels.items)[ch->index]->index = ch->index;

	ch->index = -1;
	return 0;
}

static int poll_reactor__mod(union reactor *reactor, int fd, struct esma_channel *ch, u32 events)
{
	struct pollfd *ev = esma_array_n(&reactor->poll.event_list, ch->index);

	if (unlikely(ch->index < 0))
		return 1;

	__mod__(ev, events);
	return 0;
}

static void poll_reactor__wait(union reactor *reactor)
{
/// union reactor in struct esma_reactor; struct esma_reactor in esma_engine
#define ENGINE container_of(                                            \
		container_of(reactor, struct esma_reactor, reactor),    \
		struct esma_engine,                                     \
		reactor)

	int ready = poll(reactor->poll.event_list.items, reactor->poll.event_list.nitems, -1);
	if (unlikely(-1 == ready)) {
		if (unlikely(EINTR == errno))
			return;

		esma_reactor_log_sys("%s() - poll(nevents: %d) failed: %s\n", __func__, reactor->poll.event_list.nitems, strerror(errno));
		exit(1);
	}

	for (int i = 0; i < reactor->poll.event_list.nitems && ready; i++) {
		struct pollfd *pfd = esma_array_n(&reactor->poll.event_list, i);
		struct esma_channel **ch = esma_array_n(&reactor->poll.channels, i);
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

		msg = ENGINE->queue.ops.put(&ENGINE->queue.queue);
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

api_definition(reactor_ops, reactor_poll) {
	.init = poll_reactor__init,
	.fini = poll_reactor__fini,

	.add = poll_reactor__add,
	.del = poll_reactor__del,
	.mod = poll_reactor__mod,

	.wait = poll_reactor__wait,
};
