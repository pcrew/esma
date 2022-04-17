
#include "core/esma_logger.h"

#include "engine/esma.h"

int ring_buffer__init(union queue *queue, u32 lock_type)
{
	switch (lock_type) {
	case ESMA_QUEUE_WITHOUT_LOCK:
		lock_type = ESMA_RING_BUFFER_WITHOUT_LOCK;
		break;

	case ESMA_QUEUE_MUTEX:
		lock_type = ESMA_RING_BUFFER_MUTEX;
		break;

	case ESMA_QUEUE_CMPXCHG:
		lock_type = ESMA_RING_BUFFER_CMPXCHG;
		break;

	default:
		esma_engine_log_ftl("%s() - invalid lock type: '%d'\n", __func__, lock_type);
		exit(1);
		break;
	}

	return esma_ring_buffer_init(&queue->ring_buffer, sizeof(struct esma_message), 128, lock_type);
}

void ring_buffer__fini(union queue *queue)
{
	esma_ring_buffer_free(&queue->ring_buffer);
}

void *ring_buffer__put(union queue *queue)
{
	return esma_ring_buffer_put(&queue->ring_buffer);
}

void *ring_buffer__get(union queue *queue)
{
	return esma_ring_buffer_get(&queue->ring_buffer);
}

api_definition(msg_queue_ops, msg_queue_ring_buffer) {
	.init = ring_buffer__init,
	.fini = ring_buffer__fini,
	.put = ring_buffer__put,
	.get = ring_buffer__get,
};
