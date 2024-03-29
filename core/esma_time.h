/**
 * @file
 * Copyright 2019 - present, Dmitry Lotakov
 *
 * This source code is licensed under the BSD-3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ESMA_TIME_H
#define ESMA_TIME_H

#include "common/numeric_types.h"
#include <time.h>

struct esma_time {
	time_t sec;
	u32   usec;
};

extern volatile struct esma_time *esma_time;
extern volatile u8 *esma_log_time;
extern volatile u32 esma_log_time_size;

void esma_time_init(void);
void esma_time_update(void);

#endif
