
#ifndef ESMA_TX_H
#define ESMA_TX_H

#include "common/numeric_types.h"

#include "engine/esma_engine.h"

#include "core/esma_objpool.h"

int  esma_tx_init(struct esma *tx, char *name, char *tmpl_path, int ngn_id);
void esma_tx_run(struct esma *tx, struct esma_objpool *restroom);

#endif
