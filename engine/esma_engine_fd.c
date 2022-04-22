/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <errno.h>
#include <signal.h>
#include <sys/timerfd.h>
#include <sys/signalfd.h>

#include "esma.h"
#include "core/esma_logger.h"

extern int errno;

int esma_engine_new_signalfd(int signo)
{
	sigset_t mask;
	int fd;

	sigemptyset(&mask);
	sigaddset(&mask, signo);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	fd = signalfd(-1, &mask, SFD_CLOEXEC);
	if (-1 == fd) {
		esma_engine_log_sys("%s() - signalfd(SFD_CLOEXEC): failed('%s')\n", __func__, strerror(errno));
		return -1;
	}

	return fd;
}

int esma_engine_new_timerfd(void)
{
	int fd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
	
	if (-1 == fd) {
		esma_engine_log_sys("%s() - timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC): failed('%s')\n",
			__func__, strerror(errno));
	}

	return fd;
}

int esma_engine_arm_timerfd(int fd, u64 interval_msec, int type)
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
		esma_engine_log_sys("%s() - timerfd_settime(%d, %d msec): failed('%s')\n",
				__func__, fd, interval_msec, strerror(errno));
	}

	return ret;
}

int esma_engine_disarm_timerfd(int fd)
{
	struct itimerspec itspec = {0};
	int ret = timerfd_settime(fd, 0, &itspec, NULL);

	if (-1 == ret) {
		esma_engine_log_sys("%s() - timerfd_settime(%d, 0): failed('%s')\n", __func__, fd, strerror(errno));
	}

	return ret;
}
