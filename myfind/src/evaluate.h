#ifndef EVALUATE_H
#define EVALUATE_H

#include "lexer.h"
#include "myfind.h"
#include "shunting_yard.h"

bool evaluate(char *file, struct node *ast, int *params[2]);
void eval_execplus(struct node *ast, int *err);

#endif /* ! EVALUATE_H */
