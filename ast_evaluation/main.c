#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "expression.h"
#include "parser.h"

int eval_expr(struct my_expr *expr);

int main(int argc, char **argv)
{
    static struct my_expr *expr = NULL;

    if (argc < 2)
        errx(1, "Usage: %s <token> ([token] ...)", argv[0]);

    unsigned args_length = argc - 1;
    expr = parse_expr(argv + 1, args_length);

    printf("%d\n", eval_expr(expr));

    free_expr(expr);
    return 0;
}
