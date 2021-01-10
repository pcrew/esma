
#include <errno.h>
#include <dlfcn.h>
#include <stdlib.h>

#include "load_tool.h"

void *load_tool(void *handle, char *symname)
{
	void *tool;
	char *err;

	tool = dlsym(handle, symname);
	 err = dlerror();

	if (err) {
		return NULL;
	}

	return tool;
}
