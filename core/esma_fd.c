/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "esma_fd.h"
#include "esma_logger.h"

#include "common/compiler.h"

int esma_fd_set_nonblocking(int fd, int yes)
{
	int flags = fcntl(fd, F_GETFL, 0);
	int err;

	if (unlikely(flags < 0)) {
		esma_core_log_sys("%s() - fcntl(fd: '%d', F_GETFL, 0) failed: %s\n",
				__func__, fd, strerror(errno));
		return 1;
	}

	if (yes) {
		flags |= O_NONBLOCK;
	} else {
		flags &= ~O_NONBLOCK;
	}

	err = fcntl(fd, F_SETFL, flags);
	if (unlikely(err < 0)) {
		esma_core_log_sys("%s() - fcntl(fd: '%d', F_SETFL, flags: '%d') failed: %s\n",
				__func__, fd, flags, strerror(errno));
		return 1;
	}

	return 0;
}

int esma_fd_set_closexec(int fd)
{
	int flags = fcntl(fd, F_GETFD, 0);
	int err;
	
	if (unlikely(flags < 0)) {
		esma_core_log_sys("%s() - fcntl(fd: '%d', F_GETFD, 0) failed: %s\n",
				__func__, fd, strerror(errno));
		return 1;
	}

	if (flags < 0)
		return 1;

	flags |= FD_CLOEXEC;

	err = fcntl(fd, F_SETFD, flags);
	if (unlikely(err < 0)) {
		esma_core_log_sys("%s() - fcntl(fd: '%d', F_SETFD, flags: '%d') failed: %s\n",
				__func__, fd, flags, strerror(errno));
		return 1;
	}

	return 0;
}
