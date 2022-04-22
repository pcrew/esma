
#ifndef LOAD_TOOL_H
#define LOAD_TOOL_H

#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdlib.h>

/**
 * @brief Loads tool.
 * @param [in] handle	Pointer to the library handler.
 * @param [in] symname	Name of tool.
 * @return Pointer to tool or NULL.
 */
void *load_tool(void *handle, char *symname);

#endif
