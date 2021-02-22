
#ifndef ESMA_STACK_H
#define ESMA_STACK_H

#include "common/numeric_types.h"

struct esma_stack {
	u32 top;
	u32 size;
	u32 capacity;
	u32 item_size;

	void *data;
};

struct esma_stack *esma_stack_new(u32 capacity, u32 item_size);
   int esma_stack_init(struct esma_stack *stack, u32 capacity, u32 item_size);

static inline void *esma_stack_peek(struct esma_stack *stack)
{
	return stack->size ? NULL : stack->data + stack->top * stack->item_size ;
}

void *esma_stack_push(struct esma_stack *stack);
void *esma_stack_pop(struct esma_stack *stack);

#endif
