
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "engine/esma.h"
#include "common/api.h"

struct esma_engine_info ei = {0};

int main()
{
	struct reactor *reactor = NULL;
	   int tfd;
	   int err;

	reactor = get_api("reactor_epoll");
	if (NULL == reactor)
		return 0;

	reactor->init(32, &ei);
	printf("%s() - get_api - success\n", __func__);

	tfd = reactor->new_timerfd();
	if (-1 == tfd)
		return 0;

	printf("%s() - reactor->new_timerfd() - success\n", __func__);

	reactor->add(tfd, NULL);

	reactor->arm_timerfd(tfd, 10000, ESMA_TM_PERIODIC);
//	printf("%s() - ESMA_POLLIN: %d\n", __func__, ESMA_POLLIN);
	reactor->mod(tfd, NULL, ESMA_POLLIN);

	reactor->wait();
	return 0;
}
