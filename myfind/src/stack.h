#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include <stdlib.h>

#include "myfind.h"
#include "shunting_yard.h"

struct stack
{
    struct node *data;
    struct stack *next;
};

size_t stack_size(struct stack *s);
int stack_is_empty(struct stack *s);
void stack_free(struct stack *s);
struct stack *stack_push(struct stack *s, struct node *e);
struct stack *stack_pop(struct stack *s);
struct node *stack_peek(struct stack *s);

#endif /* !STACK_H */
