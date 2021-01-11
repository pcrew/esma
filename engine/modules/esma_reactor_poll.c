
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <poll.h>

#include <sys/timerfd.h>
#include <sys/signalfd.h>

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
	if (NULL == event_list) {
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
	}
	if (err) {
		esma_engine_log_err("%s() - can't expand event_list\n", __func__);
		return 1;
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

static int poll_reactor__new_sig(int sig)
{
	sigset_t mask;
	     int fd;

	sigemptyset(&mask);
	sigaddset(&mask, sig);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	fd = signalfd(-1, &mask, SFD_CLOEXEC);
	if (-1 == fd) {
		esma_engine_log_err("%s() - signalfd(SFD_CLOEXEC): failed\n", __func__);
		return -1;
	}

        return fd;
}

static int poll_reactor__new_timerfd(void)
{
	int fd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
	if (-1 == fd) {
		esma_engine_log_err("%s() - timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC): failed\n",
				__func__);
		return -1;
	}

	return fd;
}

static int poll_reactor__arm_timerfd(int fd, int interval_msec, int type)
{
	struct itimerspec ts = {0};
	int ret;

	ts.it_value.tv_sec = interval_msec / 1000;
	ts.it_value.tv_nsec = (interval_msec % 1000) * 1000000;

	if (ESMA_TM_PERIODIC == type) { 
		ts.it_interval.tv_sec = interval_msec / 1000;
		ts.it_interval.tv_nsec = (interval_msec % 1000) * 1000000;
	}

	ret = timerfd_settime(fd, 0, &ts, NULL);
	if (-1 == ret) {
		esma_engine_log_err("%s() - timerfd_settime(%d, %d msec): failed\n",
				__func__, fd, interval_msec);
		return 1;
	}

	return 0;
}
static int poll_reactor__disarm_timerfd(int fd)
{
	struct itimerspec itspec = {0};
	int ret;

	ret = timerfd_settime(fd, 0, &itspec, NULL);
	if (-1 == ret) {
		esma_engine_log_err("%s() - timerfd_settime(%d, 0): failed\n", __func__, fd);
		return 1;
	}

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

		if (-1 == event_list[i].fd)
			continue;

		revents = event_list[i].revents;

		if (revents & POLLIN)
			e |= ESMA_POLLIN;

		if (revents & POLLOUT)
			e |= ESMA_POLLOUT;

		if (revents & POLLERR)
			e |= ESMA_POLLERR;
		
		if (revents & POLLHUP)
			e |= ESMA_POLLHUP;	

		if (0 == e)
			continue;

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

	.new_sig = poll_reactor__new_sig,

	.   new_timerfd = poll_reactor__new_timerfd,
	.   arm_timerfd = poll_reactor__arm_timerfd,
	.disarm_timerfd = poll_reactor__disarm_timerfd,

	.wait = poll_reactor__wait,
};
