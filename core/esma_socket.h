/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_SOCKET_H
#define ESMA_SOCKET_H

#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "esma_dbuf.h"

#include "common/numeric_types.h"

/**
 * @brief Structure to represent socket.
 */
struct esma_socket {
	int			fd;		/**< File descriptor. */
	int			type;		/**< Type. */
	int			family;		/**< Family. */

	/**
	* @brief Union to represent socket addres.
	*/
	union {
		struct sockaddr		sa;	/**< Sockaddr. */
		struct sockaddr_in	sa_in;	/**< Sockaddr in. */
		struct sockaddr_un	sa_un;	/**< Sockaddr un. */
		struct sockaddr_in6	sa_in6;	/**< Sockaddr in6. */
	} addr;
	socklen_t		addr_len;	/**< Sockaddr length. */
};

/**
 * @brief Creates a new socket.
 * @return Pointer to the created socket or NULL.
 */
struct esma_socket *esma_socket_new();

/**
 * @brief Initialises socket.
 * @param [out] sock	Pointer to the socket.
 * @return 0 if socket initilised successfuly or 1.
 */
int esma_socket_init(struct esma_socket *sock);

/**
 * @brief Clears socket.
 * @param [out] sock	Pointer to the socket.
 * @return 0 if socket clears successfuly or 1.
 */
int esma_socket_clear(struct esma_socket *sock);
int esma_socket_create(struct esma_socket *sock, u16 family);
int esma_socket_reset(struct esma_socket *sock);
int esma_socket_close(struct esma_socket *sock);
int esma_socket_shutdown(struct esma_socket *sock, int how);
int esma_socket_accept(struct esma_socket *client, struct esma_socket *server);
int esma_socket_bind(struct esma_socket *sock, u16 port, struct esma_dbuf *listen);
int esma_socket_listen(struct esma_socket *sock, int backlog);
int esma_socket_connect(struct esma_socket *sock);

#endif
