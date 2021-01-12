
#ifndef ESMA_REACTOR_H
#define ESMA_REACTOR_H

#include "esma_channel.h"

#include "common/numeric_types.h"
#include "common/api.h"

api_declaration(reactor) {

	void (*init)(u32 nevent, void *tools);
	void (*fini)(void);

	 int (*add)(int fd, struct esma_channel *ch);
	 int (*del)(int fd, struct esma_channel *ch);
	 int (*mod)(int fd, struct esma_channel *ch, u32 events);

	void (*wait)();
};

#endif
