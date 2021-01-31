
#ifndef ESMA_RX_H
#define ESMA_RH_H

#include "common/numeric_types.h"
#include "engine/esma_engine.h"
#include "core/esma_objpool.h"
#include "esma_sm_data.h"

int  esma_listener_init(struct esma *listener, char *name, char *tmpl_path, int ngn_id);
void esma_listener_run(struct esma *listener);

#endif
