
#include <errno.h>
#include <dlfcn.h>
#include <stdlib.h>

#include "load_tool.h"

void *load_tool(void *handle, char *symname)
{
	void *tool = dlsym(handle, symname);

	return dlerror() ? NULL : tool;
}
