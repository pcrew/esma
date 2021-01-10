
#include <fcntl.h>

#include "esma_fd.h"

int esma_fd_set_nonblocking(int fd, int yes)
{
	int err;
	int flags = 0;

	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		return 1;

	if (yes) {
		flags |= O_NONBLOCK;
	} else {
		flags &= ~O_NONBLOCK;
	}

	err = fcntl(fd, F_SETFL, flags);
	if (err < 0)
		return 1;

	return 0;
}

int esma_fd_set_closexec(int fd)
{
	int err;
	int flags = 0;

	flags = fcntl(fd, F_GETFD, 0);
	if (flags < 0)
		return 1;

	flags |= FD_CLOEXEC;

	err = fcntl(fd, F_SETFD, flags);
	if (err < 0)
		return 1;

	return 0;
}
