
#ifndef ESMA_MESSAGE_H
#define ESMA_MESSAGE_H

#include "common/numeric_types.h"
#include "esma_engine.h"

struct esma_message {
	struct esma *src;
	struct esma *dst;

	  void *ptr;
	   u32 code;
};

#endif
