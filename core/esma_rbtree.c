
#include <stdlib.h>
#include <unistd.h>

#include "esma_rbtree.h"
#include "esma_logger.h"

int esma_rbtree_init(struct esma_rbtree *tree, int (*cmp)(const void *left, const void *right))
{
	if (unlikely(NULL == tree) || (NULL == cmp))
		return 1;

	tree->root = NULL;
	tree->sentinel = NULL;

	tree->cmp = cmp;
	return 0;
}

int esma_rbtree_free(struct esma_rbtree *tree)
{

	return 0;
}

struct esma_rbtree_node *esma_rbtree_find(struct esma_rbtree *tree, void *key)
{
	struct esma_rbtree_node *node;

	if (unlikely(NULL == tree || NULL == key)) {
		esma_core_log_err("%s() - tree or key is NULL\n", __func__);
		return NULL;
	}

	if (unlikely(esma_rbtree_is_empty(tree))) {
		return NULL;
	}

	node = tree->root;

	while (node) {
		int cmp = tree->cmp(key, node->key);
		if (cmp < 0) {
			node = node->left;
			continue;
		}

		if (cmp > 0) {
			node = node->right;
			continue;
		}

		break;
	}

	return node;
}

struct esma_rbtree_node **esma_rbtree_ins(struct esma_rbtree *tree, void *key)
{
	struct esma_rbtree_node *node;
	struct esma_rbtree_node **ret = NULL;
	int sentinel = 1;

	if (unlikely(NULL == tree || NULL == key)) {
		esma_core_log_err("%s() - tree or key is NULL\n", __func__);
		return NULL;
	}

	if (unlikely(esma_rbtree_is_empty(tree))) {
		return &tree->root;
	}

	node = tree->root;
	while (node) {
		int cmp = tree->cmp(key, node->key);

		if (cmp == 0) {
			*ret = NULL;	/* duplicate key */
			goto __done;
		}

		if (cmp < 0)
			goto __left;

		goto __right;

	__left:
		sentinel = 0;
		if (node->left == NULL) {
			ret = &node->left;
			goto __done;
		}

		node = node->left;
		continue;

	__right:
		if (node->right == NULL) {
			ret = &node->right;
			goto __done;
		}

		node = node->right;
		continue;
	}

__done:
	if (sentinel) {
		tree->sentinel = *ret;
	}
	return ret;
}
