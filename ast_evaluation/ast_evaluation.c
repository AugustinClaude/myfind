#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expression.h"

int eval_expr(struct my_expr *expr)
{
    if (expr->type == EXPR_NUMBER)
        return expr->data.value;

    if (expr->type == EXPR_NEGATION)
    {
        int val = eval_expr(expr->data.children.left);
        return -val;
    }

    int l = eval_expr(expr->data.children.left);
    if (expr->type == EXPR_ADDITION)
    {
        int r = eval_expr(expr->data.children.right);
        return l + r;
    }

    if (expr->type == EXPR_SUBTRACTION)
    {
        int r = eval_expr(expr->data.children.right);
        return l - r;
    }

    if (expr->type == EXPR_MULTIPLICATION)
    {
        int r = eval_expr(expr->data.children.right);
        return l * r;
    }

    if (!expr->data.children.right)
    {
        fprintf(stderr, "Division by zero not allowed!");
        exit(1);
    }

    int r = eval_expr(expr->data.children.right);
    if (r == 0)
    {
        fprintf(stderr, "Division by zero not allowed!");
        exit(1);
    }

    return l / r;
}
