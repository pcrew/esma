
#include <unistd.h>
#include <signal.h>
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

struct esma_pollfd {
	int fd;
	void *dptr;

	struct pollfd *pollfd;
};

static struct esma_engine_info *ei = NULL;

static struct esma_pollfd *esma_fds = NULL; 
static struct pollfd *fds = NULL;

static void poll_reactor__init(u32 nevent, void *tools)
{
	if (NULL != ei || NULL != fds) {
		esma_engine_log_ftl("%s() - double reactor initialization\n", __func__);
		exit(0);
	}

	ei = tools;

	esma_fds = esma_malloc(sizeof(struct esma_pollfd) * nevent);
	if (NULL == esma_fds) {
		esma_engine_log_ftl("%s() - esma_malloc('esma_fds', %d): failed\n",
				__func__, nevent);
		exit(0);
	}

	fds = esma_malloc(sizeof(struct pollfd) * nevent);
	if (NULL == fds) {
		esma_engine_log_ftl("%s() - esma_malloc('fds', %d): failed\n",
				__func__, nevent);
		exit(0);
	}
}

static void poll_reactor__fini(void)
{
	esma_free(fds);
}

api_definition(reactor, reactor_poll) {
	.init = poll_reactor__init,
	.fini = poll_reactor__fini,
};
