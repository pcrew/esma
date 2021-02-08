
#ifndef WORKER_H
#define WORKER_H

struct worker_info {
	struct esma *manager;

	struct esma_io_context io_ctx;
	struct esma_objpool *rx_pool;
	struct esma_objpool *tx_pool;

	struct esma_dbuf dbuf;

	struct esma_socket *socket;
	struct esma_mempool *socket_pool;
};

#endif
