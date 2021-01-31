
#ifndef ESMA_SM_DATA_H
#define ESMA_SM_DATA_H

#include "core/esma_dbuf.h"

#include "common/numeric_types.h"

/* esma_rx/esma_tx data */
typedef int (*read_done_f)(struct dbuf *dbuf);

struct esma_io_context {
	struct esma_dbuf *buf;
	   int timeout_wait;
   	   int timeout_after_read;	   

	read_done_f read_done;
};

#endif
