
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

#include "../esma_channel.h"
#include "../esma_message.h"
#include "../esma_reactor.h"

#include "../esma_engine_info.h"
#include "../esma_engine_common.h"


static struct esma_engine_info *ei = NULL;

struct esma_channel **esma_channels = NULL;
static struct pollfd *event_list = NULL;
static u32 nevents;
static u32 max_events;

static void poll_reactor__init(u32 nev, void *tools)
{
	if (NULL != ei || NULL != event_list) {
		esma_engine_log_ftl("%s() - double reactor initialization\n", __func__);
		exit(0);
	}

	ei = tools;

	esma_channels = esma_malloc(sizeof(struct esma_channels *) * nev);
	if (NULL == esma_channels) {
		esma_engine_log_ftl("%s() - esma_malloc('esma_channels', %d): failed\n",
				__func__, nev);
	}

	event_list = esma_malloc(sizeof(struct pollfd) * nev);
	if (NULL == event_list) {
		esma_engine_log_ftl("%s() - esma_malloc('fds', %d): failed\n",
				__func__, nev);
		exit(0);
	}

	max_events = nev;
}

static void poll_reactor__fini(void)
{
	esma_free(event_list);
}

static int __poll_reactor_expand(void)
{
	struct pollfd *events = NULL;
	   int new_max_events = max_events << 1;

	events = esma_realloc(events, new_max_events * sizeof(struct pollfd));
	if (NULL == events) {
		return 1;
	}

	event_list = events;
	max_events = new_max_events;
	return 0;
}

static int poll_reactor__add(int fd, struct esma_channel *ch)
{
	int err = 0;
	if (nevents == max_events) {
		err = __poll_reactor_expand();
		if (err) {
			esma_engine_log_err("%s() - can't expand event_list\n", __func__);
			return 1;
		}
	}

	event_list[nevents].fd = fd;
	esma_channels[nevents] = ch;

	ch->index = nevents;
	nevents++;

	return 0;
}

static int poll_reactor__del(int fd, struct esma_channel *ch)
{
	if (ch->index < 0)
		return 1;

	nevents--;
	
	event_list[ch->index] = event_list[nevents];
	esma_channels[ch->index] = esma_channels[nevents];

	ch->index = -1;
	return 0;
}

static int poll_reactor__mod(int fd, struct esma_channel *ch, u32 event)
{
	u32 e = 0;

	if (ch->index < 0)
		return 1;

	if (event & ESMA_POLLIN) {
		e |= POLLIN;
	}

	if (event & ESMA_POLLOUT) {
		e |= POLLOUT;
	}

	event_list[ch->index].events = e;

	return 0;
}

static void poll_reactor__wait(void)
{
	struct esma_ring_buffer *msg_queue = &ei->msg_queue;
	int ready;

	ready = poll(event_list, nevents, -1);
	
	if (-1 == ready) {
		if (EINTR == errno)
			return;

		esma_engine_log_ftl("%s() - poll error: %s\n", __func__, strerror(errno));
		exit(1);
	}

	for (int i = 0; i < nevents && ready; i++) {
		struct esma_message *msg;
		u32 revents;
		u32 e = 0;

		if (-1 == event_list[i].fd || 0 == event_list[i].revents)
			continue;

		revents = event_list[i].revents;

		if (revents & POLLIN)
			e |= ESMA_POLLIN;
		else if (revents & POLLOUT)
			e |= ESMA_POLLOUT;
		else if (revents & POLLERR)
			e |= ESMA_POLLERR;
		else if (revents & POLLHUP)
			e |= ESMA_POLLHUP;	

		msg = esma_ring_buffer_put(msg_queue);
		if (NULL == msg) {
			esma_engine_log_ftl("%s() - can't put msg\n", __func__);
			exit(1);
		}

		msg->src = NULL;
		msg->dst = NULL;
		msg->ptr = esma_channels[i];
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
