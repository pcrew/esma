
#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "common/compiler.h"
#include "common/macro_magic.h"

#include "core/esma_fd.h"
#include "core/esma_logger.h"
#include "core/esma_mempool.h"
#include "core/esma_ring_buffer.h"

#include "engine/esma.h"

extern int errno;

static int epoll_reactor__init(union reactor *reactor, u32 nevents)
{
	reactor->epoll.epollfd = epoll_create(nevents);
	if (-1 == reactor->epoll.epollfd) {
		esma_reactor_log_sys("%s() - epoll_create(nevents: '%d') failed: %s\n", __func__, nevents, strerror(errno));
		return 1;
	}

	return 0;
}

static void epoll_reactor__fini(union reactor *reactor)
{
	int err = close(reactor->epoll.epollfd);

	if (-1 == err) {
		esma_reactor_log_sys("%s() - close('epollfd') failed: %s\n", __func__, strerror(errno));
		exit(1);
	}
}

ESMA_INLINE static int __mod__(union reactor *reactor, int fd, struct epoll_event *ev, u32 events)
{
	int err;
	u32 e = 0;

	if (events & ESMA_POLLIN) {
		e |= EPOLLIN;
	}

	if (events & ESMA_POLLOUT) {
		e |= EPOLLOUT;
	}

	ev->events = e;

	err = epoll_ctl(reactor->epoll.epollfd, EPOLL_CTL_MOD, fd, ev);
	if (unlikely(-1 == err)) {
		esma_reactor_log_sys("%s() - epoll_ctL(EPOLL_CTL_MOD, %d) failed: %s\n",
				__func__, fd, strerror(errno));
		return 1;
	}

	return 0;
}

static int epoll_reactor__add(union reactor *reactor, int fd, struct esma_channel *ch, u32 events)
{
	struct epoll_event ev = { .data.ptr = ch };
	int err = epoll_ctl(reactor->epoll.epollfd, EPOLL_CTL_ADD, fd, &ev);
	if (unlikely(-1 == err)) {
		esma_reactor_log_err("%s() - epoll_ctl(%d, EPOLL_CTL_ADD, %d) failed: %s\n",
				reactor->epoll.epollfd, __func__, fd, strerror(errno));
		return 1;
	}

	err = esma_fd_set_nonblocking(fd, 1);
	if (unlikely(err)) {
		esma_reactor_log_err("%s() - esma_fd_set_nonblocking(%d, true): failed\n",
				__func__, fd);
		return 1;
	}

	if (events)
		return __mod__(reactor, fd, &ev, events);

	return 0;
}

static int epoll_reactor__del(union reactor *reactor, int fd, __attribute__((unused)) struct esma_channel *ch)
{
	struct epoll_event ev = {0};
	int err = epoll_ctl(reactor->epoll.epollfd, EPOLL_CTL_DEL, fd, &ev);
	if (unlikely(-1 == err)) {
		esma_reactor_log_sys("%s() - epoll_ctl(%d, EPOLL_CTL_DEL, %d): %s\n",
				__func__, reactor->epoll.epollfd, fd, strerror(errno));
		return 1;
	}

	return 0;
}

static int epoll_reactor__mod(union reactor *reactor, int fd, struct esma_channel *ch, u32 events)
{
	struct epoll_event ev = { .data.ptr = ch };
	return __mod__(reactor, fd, &ev, events);
}

#define MAX_EVENTS 32
static struct epoll_event events[MAX_EVENTS];

static void epoll_reactor__wait(union reactor *reactor)
{
#define ENGINE container_of(							\
			container_of(reactor, struct esma_reactor, reactor),	\
			struct esma_engine,					\
			reactor)

	int nfds = epoll_wait(reactor->epoll.epollfd, events, MAX_EVENTS, -1);
	if (unlikely(-1 == nfds)) {
		esma_reactor_log_sys("%s() - epoll_wait(%d, EVENTS[%d]) failed: %s\n",
				__func__, reactor->epoll.epollfd, MAX_EVENTS, strerror(errno));
		return; /* exit(1) ?  */
	}

	for (int i = 0; i < nfds; i++) {
		struct esma_message *msg = ENGINE->queue.ops.put(&ENGINE->queue.queue);
		u32 e = 0;

		if (unlikely(NULL == msg)) {
			esma_reactor_log_err("%s() - can't put msg\n", __func__);
			exit(1);
		}

		if (events[i].events & EPOLLIN)
			e |= ESMA_POLLIN;
		else if (events[i].events & EPOLLOUT)
			e |= ESMA_POLLOUT;
		else if (events[i].events & EPOLLERR)
			e |= ESMA_POLLERR;
		else if (events[i].events & EPOLLHUP)
			e |= ESMA_POLLHUP;

		msg->src = NULL;
		msg->dst = NULL;
		msg->ptr = events[i].data.ptr;
		msg->code = e;
	}
}

api_definition(reactor_ops, reactor_epoll) {

	.init = epoll_reactor__init,
	.fini = epoll_reactor__fini,

	.add = epoll_reactor__add,
	.del = epoll_reactor__del,
	.mod = epoll_reactor__mod,

	.wait = epoll_reactor__wait,
};

