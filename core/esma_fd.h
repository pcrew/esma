/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_FD_H
#define ESMA_FD_H

/**
 * @brief Makes the file descriptor available (or not) for non-blocking operations.
 * @param [in] fd File descriptor.
 * @param [in] yes 0 - no, otherwise - yes.
 * @return 0 - if file discriptor maked non_blocking successfuly; 1 - otherwise.
 */
int esma_fd_set_nonblocking(int fd, int yes);

/**
 * @brief Makes the file descriptor available for closexec.
 * @param [in] fd File descriptor.
 * @return 0 - if file discriptor maked closexec successfuly; 1 - otherwise.
 */
int esma_fd_set_closexec(int fd);

#endif
