#ifndef SHUNTING_YARD_H
#define SHUNTING_YARD_H

#include "lexer.h"
#include "myfind.h"
#include "stack.h"

void free_ast(struct node *ast);
char **copy_exec(char **exec);
struct node *shunting_yard(struct node **nodes);

#endif /* ! SHUNTING_YARD_H */
