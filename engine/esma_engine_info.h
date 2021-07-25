
#ifndef ESMA_ENGINE_INFO
#define ESMA_ENGINE_INFO

#include "core/esma_ring_buffer.h"

struct esma_engine_info {
	   int index;
	   int status;
	struct esma_ring_buffer msg_queue;
};

#endif
