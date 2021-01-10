
#ifndef API_H
#define API_H

#define api_declaration(stag) struct stag
#define api_definition(stag,var) struct stag var =

#include "utils/load_tool.h"

static inline void *get_api(char *name)
{
	return load_tool(NULL, name);
}

#endif
