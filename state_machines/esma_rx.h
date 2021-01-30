
#ifndef ESMA_RX_H
#define ESMA_RH_H

#include "common/numeric_types.h"
#include "engine/esma_engine.h"
#include "core/esma_objpool.h"

struct esma_message_to_rx {
	struct esma_dbuf *dbuf;
	   u32 timeout_after_read;
	   u32 timeout_wait;

	   int (*done)(struct dbuf *dbuf);
}

int esma_rx_init(struct esma *rx, char *name, char *tmpl_path, int ngn_id);
int esma_rx_run(struct esma *rx, struct esma_objpool *restroom);

#endif
