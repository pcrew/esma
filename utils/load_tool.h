
#ifndef LOAD_TOOL_H
#define LOAD_TOOL_H

#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdlib.h>

void *load_tool(void *handle, char *symname);

#endif
