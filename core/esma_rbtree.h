
#ifndef ESMA_RBTREE_H
#define ESMA_RBTREE_H

#include "esma_mempool.h"

#include "common/numeric_types.h"
#include "common/compiler.h"

struct esma_rbtree_node {
	struct esma_rbtree_node *left;
	struct esma_rbtree_node *right;
	struct esma_rbtree_node *parent;
	u32 color;
	void *key;
};

struct esma_rbtree {
	struct esma_rbtree_node *root;
	struct esma_rbtree_node *sentinel;

	int (*cmp)(const void *left, const void *right);
};

int esma_rbtree_init(struct esma_rbtree *tree, int (*cmp)(const void *left, const void *right));
int esma_rbtree_free(struct esma_rbtree *tree);

struct esma_rbtree_node  *esma_rbtree_find(struct esma_rbtree *tree, void *key);
struct esma_rbtree_node **esma_rbtree_ins(struct esma_rbtree *tree, void *key);

static inline int esma_rbtree_is_empty(struct esma_rbtree *tree)
{
	if (NULL == tree)
		return 1;

	return !!(tree->root == NULL);
}

#endif
