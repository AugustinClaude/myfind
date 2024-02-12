#ifndef LEXER_H
#define LEXER_H

#include "myfind.h"

struct node *node_and(void);
int get_nodes_len(struct node **nodes);
void free_nodes(struct node **nodes);
struct node **init_nodes_list(int len);
struct node **lexer(int argc, char **argv);

#endif /* ! LEXER_H */
