
#ifndef ESMA_SM_DATA_H
#define ESMA_SM_DATA_H

#include "core/esma_dbuf.h"

#include "common/numeric_types.h"

/* esma_rx/esma_tx data */
typedef int (*read_done_f)(struct esma_dbuf *dbuf);

struct esma_io_context {
	int			 fd;
	struct esma_dbuf	*dbuf;
	int			 timeout_wait;
   	int			 timeout_after_recv;	   

	read_done_f		 read_done;
};

#endif
