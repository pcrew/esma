
#ifndef ESMA_SOCKET_H
#define ESMA_SOCKET_H

#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "esma_dbuf.h"

#include "common/numeric_types.h"

struct esma_socket {
	int			fd;
	int			type;
	int			family;

	union {
		struct sockaddr		sa;
		struct sockaddr_in	sa_in;
		struct sockaddr_un	sa_un;
		struct sockaddr_in6	sa_in6;
	} addr;
	socklen_t		addr_len;
};

static inline int esma_socket_invalid(struct esma_socket *sock)
{
	if (sock->fd < 0 || sock->type < 0 || sock->family < 0)
		return 1;

	return 0;
}

struct esma_socket *esma_socket_new();
   int esma_socket_init(struct esma_socket *sock);
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
