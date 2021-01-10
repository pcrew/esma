
#ifndef ESMA_FD_H
#define ESMA_FD_H

#include "esma_dbuf.h"

int esma_fd_set_nonblocking(int fd, int yes);
int esma_fd_set_closexec(int fd);

#endif
