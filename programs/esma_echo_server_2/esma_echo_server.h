
#ifndef ESMA_ECHO_SERVER_H
#define ESMA_ECHO_SERVER_H

struct manager_ctx {
	struct esma *listener;
	struct esma_socket *socket;
};

#endif
