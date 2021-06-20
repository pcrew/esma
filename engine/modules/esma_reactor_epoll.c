
#include <unistd.h>

#include <sys/epoll.h>

#include "common/compiler.h"

#include "core/esma_fd.h"
#include "core/esma_logger.h"
#include "core/esma_mempool.h"
#include "core/esma_ring_buffer.h"

#include "../esma_channel.h"
#include "../esma_message.h"
#include "../esma_reactor.h"

#include "../esma_engine_info.h"
#include "../esma_engine_common.h"

static struct esma_engine_info *ei = NULL;
static int epollfd = -1;


static void epoll_reactor__init(u32 nevent, void *tools)
{
	if (NULL != ei || -1 != epollfd) {
		esma_reactor_log_err("%s() - double reactor initialization\n", __func__);
		exit(1);
	}

	ei = tools;

	epollfd = epoll_create(nevent);
	if (-1 == epollfd) {
		esma_reactor_log_err("%s() - epoll_create('%d'): failed\n", __func__, nevent);
		exit(1);
	}
}

static void epoll_reactor__fini(void)
{
	int err = close(epollfd);

	if (-1 == err) {
		esma_reactor_log_err("%s() - close('epollfd'): failed\n", __func__);
		exit(1);
	}
}

static int epoll_reactor__add(int fd, struct esma_channel *ch)
{
	struct epoll_event ev = {0};
	   int err;

	ev.data.fd  = fd;
	ev.data.ptr = ch;

	err = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
	if (-1 == err) {
		esma_reactor_log_err("%s() - epoll_ctl(%d, EPOLL_CTL_ADD, %d): failed\n",
				epollfd, __func__, fd);
		return 1;
	}

	err = esma_fd_set_nonblocking(fd, 1);
	if (err) {
		esma_reactor_log_err("%s() - esma_fd_set_nonblocking(%d, true): failed\n",
				__func__, fd);
		return 1;
	}

	return 0;
}

static int epoll_reactor__del(int fd, __attribute__((unused)) struct esma_channel *ch)
{
	struct epoll_event ev = {0};
	   int ret;

	ret = epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
	if (-1 == ret) {
		esma_reactor_log_err("%s() - epoll_ctl(%d, EPOLL_CTL_DEL, %d): %s\n",
				__func__, epollfd, fd, strerror(errno));
		return 1;
	}

	return 0;
}

static int epoll_reactor__mod(int fd, struct esma_channel *ch, u32 events)
{
	struct epoll_event ev = {0};
	   int ret;
	   u32 e = 0;

	if (events & ESMA_POLLIN) {
		e |= EPOLLIN;
	}

	if (events & ESMA_POLLOUT) {
		e |= EPOLLOUT;
	}

	if (events & ESMA_POLLERR) {
		e |= EPOLLERR;
	}

	if (events & ESMA_POLLHUP) {
		e |= EPOLLHUP;
	}

	ev.data.ptr = ch;
	ev.events = e;

	ret = epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
	if (-1 == ret) {
		esma_reactor_log_err("%s() - epoll_ctL(EPOLL_CTL_MOD, %d): failed\n",
				__func__, fd);
		return 1;
	}

	return 0;
}

#define MAX_EVENTS 32
static struct epoll_event events[MAX_EVENTS];

static void epoll_reactor__wait(void)
{
	struct esma_ring_buffer *msg_queue = &ei->msg_queue;
	int nfds;

	nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
	if (-1 == nfds) {
		esma_reactor_log_err("%s() - epoll_wait(%d, EVENTS[%d]): failed\n",
				__func__, epollfd, MAX_EVENTS);
		return; /* exit(1) ?  */
	}

	for (int i = 0; i < nfds; i++) {
		struct esma_message *msg;
		u32 e = 0;

		msg = esma_ring_buffer_put(msg_queue);
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

	return;
}

api_definition(reactor, reactor_epoll) {

	.init = epoll_reactor__init,
	.fini = epoll_reactor__fini,

	.add = epoll_reactor__add,
	.del = epoll_reactor__del,
	.mod = epoll_reactor__mod,

	.wait = epoll_reactor__wait,
};
