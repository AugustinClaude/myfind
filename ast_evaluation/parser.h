#ifndef PARSER_H
#define PARSER_H

#include "expression.h"

void free_expr(struct my_expr *expr);
struct my_expr *parse_expr(char **strs, unsigned length);

#endif /* ! PARSER_H */
