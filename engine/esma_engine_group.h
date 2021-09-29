
#ifndef ESMA_ENGINE_GROUP_H
#define ESMA_ENGINE_GROUP_H

#include "esma_engine.h"

struct esma_sm_group {
		
};

int esma_sm_group_init(struct esma_sm_group *group);

 int esma_sm_group_add(struct esma_sm_group *group, struct esma *esma);
 int esma_sm_group_del(struct esma_sm_group *group, struct esma *esma);
void esma_sh_group_shedule(struct esma_sm_group, u32 time_msec, int *err);

#endif
