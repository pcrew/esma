
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "esma_fd.h"
#include "esma_dbuf.h"
#include "esma_alloc.h"
#include "esma_socket.h"
#include "esma_logger.h"

#include "common/compiler.h"

int esma_socket_init(struct esma_socket *sock)
{
	if (unlikely(NULL == sock))
		return 1;

	sock->fd = -1;
	sock->type = -1;
	sock->family = -1;

	memset(&sock->addr, 0, sizeof(sock->addr));
	sock->addr_len = -1;
	return 0;
}

int esma_socket_clear(struct esma_socket *sock)
{
	return esma_socket_init(sock);
}

struct esma_socket *esma_socket_new()
{
	struct esma_socket *sock = NULL;
	   int err;

	sock = esma_malloc(sizeof(struct esma_socket));
	if (unlikely(NULL == sock)) {
		return NULL;
	}

	err = esma_socket_init(sock);
	if (unlikely(err)) {
		esma_free(sock);
		return NULL;
	}

	return sock;
}

int esma_socket_create(struct esma_socket *sock, u16 family)
{
	if (NULL == sock)
		return 1;

	sock->fd = socket(family, SOCK_STREAM, 0);
	if (unlikely(-1 == sock->fd)) {
		esma_core_log_err("%s() - socket('%d', SOCK_STREAM, 0): failed\n", __func__, family);
		return 1;
	}

	switch(family) {
	case AF_INET:
		sock->addr.sa_in.sin_family = AF_INET;
		sock->addr_len = sizeof(struct sockaddr_in);
		break;

	case AF_INET6:
		sock->addr.sa_in6.sin6_family = AF_INET6;
		sock->addr_len = sizeof(struct sockaddr_in6);
		break;

	case AF_UNIX:
		sock->addr.sa_un.sun_family = AF_UNIX;
		sock->addr_len = sizeof(struct sockaddr_un);
		break;
	}
	memset(&sock->addr, 0, sizeof(sock->addr));

	sock->family = family;
	return 0;
}

int esma_socket_reset(struct esma_socket *sock)
{
	int err;
	struct linger linger = {0};

	if (unlikely(sock->fd < 0)) {
		return 1;
	}

	linger.l_linger = 0;
	linger.l_onoff = 1;

	err = setsockopt(sock->fd, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
	if (-1 == err) {
		esma_core_log_err("%s() - setsockopt('%d', SOL_SOCKET, SO_LINGER): failed\n", __func__, sock->fd);
		return 1;
	}

	return 0;
}

int esma_socket_close(struct esma_socket *sock)
{
	int ret;

	if (unlikely(sock->fd < 0)) {
		return 1;
	}

	ret = close(sock->fd);

	sock->fd = -1;
	return ret;
}

int esma_socket_shutdown(struct esma_socket *sock, int how)
{
	int err;

	if (unlikely(sock->fd < 0)) {
		return 1;
	}

	err = shutdown(sock->fd, how);
	if (-1 == err)
		esma_core_log_err("%s() - shutdown('%d', '%d'): failed\n", __func__, sock->fd, how); 
		return 1;

	return 0;
}

int esma_socket_accept(struct esma_socket *client, struct esma_socket *server)
{
	int fd;
	int err;
	struct sockaddr sa;
	socklen_t len = server->addr_len;

	fd = accept(server->fd, &sa, &len);
	if (fd < 0) {
		esma_core_log_err("%s() - accept('%d'): failed\n", __func__, server->fd);
		return 1;
	}

	err = esma_fd_set_closexec(fd);
	if (err) {
		esma_core_log_err("%s() - esma_fd_set_closexec(%d): failed\n", __func__, fd);
		return 1;
	}

	client->fd = fd;

	switch (sa.sa_family) {
	case AF_INET:
		client->addr_len = sizeof(struct sockaddr_in);
		break;

	case AF_INET6:
		client->addr_len = sizeof(struct sockaddr_in6);
		break;
	}

	memcpy(&client->addr, &sa, client->addr_len);

	client->fd = fd;
	client->family = sa.sa_family;
	return 0;
}

int __esma_socket_bind_v4(struct esma_socket *s, u16 p, struct esma_dbuf *l)
{
	int err;

	s->addr.sa_in.sin_port = htons(p);

	if (NULL == l) {
		s->addr.sa_in.sin_addr.s_addr = INADDR_ANY;
		goto __bind;
	}

	goto __bind;

__bind:
	err = bind(s->fd, (struct sockaddr *) &s->addr, s->addr_len);
	if (err) {
		esma_core_log_err("%s() - bind('%d'): failed\n", __func__, s->fd);
		return 1;
	}

	return 0;
}

int __esma_socket_bind_v6(struct esma_socket *s, u16 p, struct esma_dbuf *l)
{
	int err;

	s->addr.sa_in6.sin6_port = htons(p);

	if (NULL == l) {
		s->addr.sa_in6.sin6_addr = in6addr_any;
		goto __bind;
	}

	goto __bind;

__bind:
	err = bind(s->fd, (struct sockaddr *) &s->addr, s->addr_len);
	if (err) {
		esma_core_log_err("%s() - bind('%d'): failed\n", __func__, s->fd);
		return 1;
	}

	return 0;
}

int __esma_socket_bind_unix(struct esma_socket *s, u16 p, struct esma_dbuf *l)
{
	struct stat st;
	int err;

	if (l->len >= sizeof(s->addr.sa_un.sun_path)) {
		esma_core_log_err("%s() - too long path name\n", __func__);
		return 1;
	}

	err = stat((char *) l->loc, &st);
	if (err)
		goto __create_socket;

	if (0 == S_ISSOCK(st.st_mode)) {
		esma_core_log_err("%s() - S_ISSOCK(): failed\n", __func__);
		return 1;
	}

	err = unlink((char *) l->loc);
	if (err) {
		esma_core_log_err("%s() - unlink(): failed\n", __func__);
		return 1;
	}

	goto __create_socket;

__create_socket:

	memcpy(s->addr.sa_un.sun_path, l->loc, l->len + 1);
	s->addr_len = sizeof(s->addr.sa_un.sun_family) + l->len;

	err = bind(s->fd, (struct sockaddr *) &s->addr.sa_un, s->addr_len);
	if (err) {
		esma_core_log_err("%s() - bind('%d'): failed\n", __func__, s->fd);
		return 1;
	}

	return 0;
}

int esma_socket_bind(struct esma_socket *sock, u16 port, struct esma_dbuf *listen)
{
	int ret;

	switch (sock->family) {
	case AF_INET:
		ret = __esma_socket_bind_v4(sock, port, listen);
		break;

	case AF_INET6:
		ret = __esma_socket_bind_v6(sock, port, listen);
		break;

	case AF_UNIX:
		ret = __esma_socket_bind_unix(sock, port, listen);
		break;

	default:
		ret = 1;
		break;
	}

	return ret;
}

int esma_socket_listen(struct esma_socket *sock, int backlog)
{
	int err;

	err = listen(sock->fd, backlog);
	if (-1 == err) {
		esma_core_log_err("%s() - listen('%d', '%d'): failed\n", __func__, sock->fd, backlog);
		return 1;
	}

	return 0;
}

int esma_socket_connect(struct esma_socket *sock)
{
	int err;

	err = connect(sock->fd, (struct sockaddr *) &sock->addr, sock->addr_len);
	if (err) {
		esma_core_log_err("%s() - connect('%d'): failed\n", __func__, sock->fd);
		return 1;
	}

	return 0;
}
