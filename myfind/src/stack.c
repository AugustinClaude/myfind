#include "stack.h"

size_t stack_size(struct stack *s)
{
    size_t len = 0;
    struct stack *tmp = s;
    while (tmp)
    {
        len++;
        tmp = tmp->next;
    }

    return len;
}

int stack_is_empty(struct stack *s)
{
    return !s || stack_size(s) == 0;
}

void stack_free(struct stack *s)
{
    struct stack *tmp = s;
    while (tmp)
    {
        struct stack *tmp2 = tmp;
        tmp = tmp->next;

        free_ast(tmp2->data);
        free(tmp2);
    }
}

struct stack *stack_push(struct stack *s, struct node *e)
{
    struct stack *head = malloc(sizeof(struct stack));
    if (!head)
    {
        return NULL;
    }

    head->data = e;
    head->next = s;

    return head;
}

struct stack *stack_pop(struct stack *s)
{
    if (!s)
    {
        return NULL;
    }

    struct stack *next = s->next;
    s->next = NULL;
    free_ast(s->data);
    free(s);

    return next;
}

struct node *stack_peek(struct stack *s)
{
    return s->data;
}
