#include "shunting_yard.h"

void free_files(char **files)
{
    if (!files)
        return;

    for (int i = 0; i < get_files_len(files); i++)
        free(files[i]);

    free(files);
}

void free_ast(struct node *ast)
{
    if (!ast)
        return;

    free_ast(ast->left);
    free_ast(ast->right);

    if (ast->type == EXEC || ast->type == EXECPLUS || ast->type == EXECDIR)
        free(ast->exec);
    if (ast->type == EXECPLUS)
        free_files(ast->files);

    free(ast);
}

static int get_prio(struct node *node)
{
    if (node->type == OR)
        return 1;
    if (node->type == AND)
        return 2;
    if (node->type == NOT)
        return 3;
    if (node->type == LPAR || node->type == RPAR)
        return 4;

    return 0;
}

char **copy_exec(char **exec)
{
    if (!exec)
        return NULL;

    char **dest = malloc(sizeof(char *) * (get_files_len(exec) + 1));

    int i;
    for (i = 0; exec[i]; i++)
        dest[i] = exec[i];
    dest[i] = NULL;

    return dest;
}

static struct node *copy(struct node *src)
{
    if (!src)
        return NULL;

    struct node *dest = malloc(sizeof(struct node));
    dest->expr_type = src->expr_type;
    dest->type = src->type;
    dest->left = copy(src->left);
    dest->right = copy(src->right);
    dest->value = src->value;
    dest->exec = copy_exec(src->exec);
    dest->files = copy_exec(src->files);
    return dest;
}

static void push_op(struct stack **out, struct stack **op, struct node *op2)
{
    if (stack_is_empty(*out))
    {
        stack_free(*out);
        stack_free(*op);
        errx(1, "Syntax Error: Missing operand.");
    }

    struct node *right = copy(stack_peek(*out));
    *out = stack_pop(*out);
    op2->right = right;

    if (op2->type != NOT)
    {
        if (stack_is_empty(*out))
        {
            stack_free(*out);
            stack_free(*op);
            errx(1, "Syntax Error: Missing operand.");
        }

        struct node *left = copy(stack_peek(*out));
        *out = stack_pop(*out);
        op2->left = left;
    }
}

static void if_operator_node(struct stack **out, struct stack **op,
                             struct node *node)
{
    while (!stack_is_empty(*op) && stack_peek(*op)->type != LPAR
           && (get_prio(stack_peek(*op)) > get_prio(node)
               || (get_prio(stack_peek(*op)) == get_prio(node)
                   && node->type != NOT)))
    {
        struct node *op2 = copy(stack_peek(*op));
        *op = stack_pop(*op);
        push_op(out, op, op2);
        *out = stack_push(*out, op2);
    }

    *op = stack_push(*op, node);
}

static void if_operator(struct stack **out, struct stack **op,
                        struct node **nodes, int i)
{
    while (!stack_is_empty(*op) && stack_peek(*op)->type != LPAR
           && (get_prio(stack_peek(*op)) > get_prio(nodes[i])
               || (get_prio(stack_peek(*op)) == get_prio(nodes[i])
                   && nodes[i]->type != NOT)))
    {
        struct node *op2 = copy(stack_peek(*op));
        *op = stack_pop(*op);
        push_op(out, op, op2);
        *out = stack_push(*out, op2);
    }

    *op = stack_push(*op, copy(nodes[i]));
}

static void if_rpar(struct stack **out, struct stack **op)
{
    while (!stack_is_empty(*op) && stack_peek(*op)->type != LPAR)
    {
        struct node *op2 = copy(stack_peek(*op));
        *op = stack_pop(*op);
        push_op(out, op, op2);
        *out = stack_push(*out, op2);
    }

    if (stack_is_empty(*op) || stack_peek(*op)->type != LPAR)
    {
        stack_free(*out);
        stack_free(*op);
        errx(1, "Syntax Error: Unbalanced parentheses.");
    }

    *op = stack_pop(*op);
}

static void handle_node(struct stack **out, struct stack **op,
                        struct node **nodes, int *i)
{
    while (nodes[*i])
    {
        if (nodes[*i]->expr_type != OPERATOR)
        {
            if (*i - 1 >= 0
                && (nodes[*i - 1]->expr_type != OPERATOR
                    || nodes[*i - 1]->type == RPAR))
                if_operator_node(out, op, node_and());

            *out = stack_push(*out, copy(nodes[*i]));
        }
        else
        {
            if (nodes[*i]->type == LPAR)
            {
                if (*i - 1 >= 0
                    && (nodes[*i - 1]->type == RPAR
                        || nodes[*i - 1]->expr_type != OPERATOR))
                    if_operator_node(out, op, node_and());

                *op = stack_push(*op, copy(nodes[*i]));
            }
            else if (nodes[*i]->type == RPAR)
                if_rpar(out, op);
            else
            {
                if (nodes[*i]->type == NOT && *i - 1 >= 0
                    && (nodes[*i - 1]->expr_type != OPERATOR
                        || nodes[*i - 1]->type == RPAR))
                    if_operator_node(out, op, node_and());

                if_operator(out, op, nodes, *i);
            }
        }

        (*i)++;
    }
}

struct node *shunting_yard(struct node **nodes)
{
    if (!nodes)
        return NULL;

    int i = 0;

    struct stack *out = NULL;
    struct stack *op = NULL;

    handle_node(&out, &op, nodes, &i);

    while (!stack_is_empty(op))
    {
        struct node *op2 = copy(stack_peek(op));
        if (op2->type == LPAR || op2->type == RPAR)
        {
            stack_free(out);
            stack_free(op);
            errx(1, "Syntax Error: Unbalanced parentheses.");
        }

        op = stack_pop(op);
        push_op(&out, &op, op2);
        out = stack_push(out, op2);
    }

    if (stack_is_empty(out) || stack_size(out) != 1)
    {
        stack_free(out);
        stack_free(op);
        errx(1, "Syntax Error: Stack size is not 1.");
    }

    struct node *ast = copy(stack_peek(out));

    stack_free(out);
    stack_free(op);

    return ast;
}
