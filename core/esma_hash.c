
#include <stdlib.h>
#include <unistd.h>

#include "esma_cpu.h"
#include "esma_hash.h"
#include "esma_alloc.h"
#include "esma_logger.h"

static u32 _hash_key(u8 *key, u32 len)
{
	u32 ret = 0;

	for (int i = 0; i < len; i++) {
		ret = ret * 32 + key[i];
	}

	return ret;
}

int esma_hashtable_init(struct esma_hashtable *table, char **keys, u32 nkeys)
{
	return 0;
}

struct esma_hashtable *esma_hashtable_new(char **keys, u32 nkeys)
{
	struct esma_hashtable *table = esma_malloc(sizeof(struct esma_hashtable));
	int err;

	if (NULL == table) {
		return NULL;
	}

	err = esma_hashtable_init(table, keys, nkeys);
	if (err) {
		esma_free(table);
		return NULL;
	}

	return table;
}
