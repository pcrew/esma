
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "esma_stack.h"
#include "esma_alloc.h"
#include "esma_logger.h"
#include "common/compiler.h"

int esma_stack_init(struct esma_stack *stack, u32 capacity, u32 item_size)
{
	void *data;

	if (NULL == stack) {
		esma_core_log_err("%s() - stack is NULL\n", __func__);
		return 1;
	}

	data = esma_malloc(item_size * capacity);
	if (NULL == data) {
		esma_core_log_err("%s() - can't allocate memory for stack data\n", __func__);
		return 1;
	}

	memset(data, 0, item_size * capacity);

	stack->top = 0;
	stack->size = 0;
	stack->capacity = capacity;
	stack->item_size = item_size;
	stack->data = data;

	return 0;
}

struct esma_stack *esma_stack_new(u32 capacity, u32 item_size)
{
	struct esma_stack *stack;
	   int err;

	stack = esma_malloc(sizeof(struct esma_stack));
	if (NULL == stack) {
		esma_core_log_err("%s() - can't allocate memory for new stack\n", __func__);
		return NULL;
	}

	err = esma_stack_init(stack, capacity, item_size);
	if (err) {
		esma_core_log_err("%s() - can't initialize stack\n", __func__);
		esma_free(stack);
		return NULL;
	}

	return stack;
}

static int _stack_expand(struct esma_stack *stack)
{
	void *data;
	 u32  new_cap = stack->capacity << 1;

	data = esma_realloc(stack->data, new_cap * stack->item_size);
	if (NULL == data) {
		esma_core_log_err("%s() - can't reallocate memory for stack data\n", __func__);
		return 1;
	}

	stack->capacity = new_cap;
	stack->data = data;
	return 0;
}

void *esma_stack_push(struct esma_stack *stack)
{
	if (unlikely(NULL == stack)) {
		esma_core_log_err("%s() - stack is NULL\n", __func__);
		return NULL;
	}

	stack->size++;
	return stack->data + stack->size * stack->item_size;
}

void *esma_stack_pop(struct esma_stack *stack)
{
	void *ret;

	if (unlikely(NULL == stack)) {
		esma_core_log_err("%s() - stack is NULL\n", __func__);
		return NULL;
	}

	if (0 == stack->size)
		return NULL;

	ret = stack->data + stack->size * stack->item_size;
	stack->size--;

	return ret;
}
