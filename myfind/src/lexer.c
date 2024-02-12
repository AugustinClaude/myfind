#include "lexer.h"

static struct node *node_print(void)
{
    struct node *node = malloc(sizeof(struct node));
    node->expr_type = ACTION;
    node->type = PRINT;
    node->left = NULL;
    node->right = NULL;
    node->value = "-print";
    node->exec = NULL;
    node->files = NULL;
    return node;
}

static struct node *node_delete(void)
{
    struct node *node = malloc(sizeof(struct node));
    node->expr_type = ACTION;
    node->type = DELETE;
    node->left = NULL;
    node->right = NULL;
    node->value = "-delete";
    node->exec = NULL;
    node->files = NULL;
    return node;
}

static struct node *node_printdel(char *value)
{
    if (strcmp(value, "-print") == 0)
        return node_print();
    return node_delete();
}

static struct node *node_name(struct node **nodes, char *value)
{
    if (!value)
    {
        free_nodes(nodes);
        errx(1, "missing argument to -name");
    }

    struct node *node = malloc(sizeof(struct node));
    node->expr_type = TEST;
    node->type = NAME;
    node->left = NULL;
    node->right = NULL;
    node->value = value;
    node->exec = NULL;
    node->files = NULL;
    return node;
}

static struct node *node_type(struct node **nodes, char *value)
{
    if (!value)
    {
        free_nodes(nodes);
        errx(1, "missing argument to -type");
    }

    struct node *node = malloc(sizeof(struct node));
    node->expr_type = TEST;
    node->type = TYPE;
    node->left = NULL;
    node->right = NULL;
    node->value = value;
    node->exec = NULL;
    node->files = NULL;
    return node;
}

static struct node *node_newer(struct node **nodes, char *value)
{
    if (!value)
    {
        free_nodes(nodes);
        errx(1, "missing argument to -newer");
    }

    struct node *node = malloc(sizeof(struct node));
    node->expr_type = TEST;
    node->type = NEWER;
    node->left = NULL;
    node->right = NULL;
    node->value = value;
    node->exec = NULL;
    node->files = NULL;
    return node;
}

static struct node *node_perm(struct node **nodes, char *value)
{
    if (!value)
    {
        free_nodes(nodes);
        errx(1, "missing argument to -perm");
    }

    struct node *node = malloc(sizeof(struct node));
    node->expr_type = TEST;
    node->type = PERM;
    node->left = NULL;
    node->right = NULL;
    node->value = value;
    node->exec = NULL;
    node->files = NULL;
    return node;
}

static struct node *node_user(struct node **nodes, char *value)
{
    if (!value)
    {
        free_nodes(nodes);
        errx(1, "missing argument to -user");
    }

    struct node *node = malloc(sizeof(struct node));
    node->expr_type = TEST;
    node->type = USER;
    node->left = NULL;
    node->right = NULL;
    node->value = value;
    node->exec = NULL;
    node->files = NULL;
    return node;
}

static struct node *node_group(struct node **nodes, char *value)
{
    if (!value)
    {
        free_nodes(nodes);
        errx(1, "missing argument to -group");
    }

    struct node *node = malloc(sizeof(struct node));
    node->expr_type = TEST;
    node->type = GROUP;
    node->left = NULL;
    node->right = NULL;
    node->value = value;
    node->exec = NULL;
    node->files = NULL;
    return node;
}

static struct node *node_exec(char **exec)
{
    struct node *node = malloc(sizeof(struct node));
    node->expr_type = ACTION;
    node->type = EXEC;
    node->left = NULL;
    node->right = NULL;
    node->value = "-exec";
    node->exec = exec;
    node->files = NULL;
    return node;
}

static struct node *node_execplus(char **exec)
{
    struct node *node = malloc(sizeof(struct node));
    node->expr_type = ACTION;
    node->type = EXECPLUS;
    node->left = NULL;
    node->right = NULL;
    node->value = "-exec+";
    node->exec = exec;
    node->files = malloc(sizeof(char *));
    node->files[0] = NULL;
    return node;
}

static struct node *handle_exec(struct node **nodes, char **argv, int *i,
                                int len)
{
    char **begin = argv + *i + 1;
    while (*i < len && argv[*i][0] != ';' && argv[*i][0] != '+')
        (*i)++;
    char **end = argv + *i;

    if (*i >= len)
    {
        free_nodes(nodes);
        errx(1, "Syntax Error: Missing ';' or '+' in '-exec'.");
    }

    char **exec = malloc(sizeof(char *));
    exec[0] = NULL;
    while (begin < end)
    {
        exec = add_files(exec, *begin);
        begin++;
    }

    if (argv[*i][0] == ';')
        return node_exec(exec);
    return node_execplus(exec);
}

static struct node *node_execdir(struct node **nodes, char **argv, int *i,
                                 int len)
{
    char **begin = argv + *i + 1;
    while (*i < len && argv[*i][0] != ';')
        (*i)++;
    char **end = argv + *i;

    if (*i >= len)
    {
        free_nodes(nodes);
        errx(1, "Syntax Error: Missing ';' in '-execdir'.");
    }

    char **exec = malloc(sizeof(char *));
    exec[0] = NULL;
    while (begin < end)
    {
        exec = add_files(exec, *begin);
        begin++;
    }

    struct node *node = malloc(sizeof(struct node));
    node->expr_type = ACTION;
    node->type = EXECDIR;
    node->left = NULL;
    node->right = NULL;
    node->value = "-execdir";
    node->exec = exec;
    node->files = NULL;
    return node;
}

static struct node *node_or(void)
{
    struct node *node = malloc(sizeof(struct node));
    node->expr_type = OPERATOR;
    node->type = OR;
    node->left = NULL;
    node->right = NULL;
    node->value = "-o";
    node->exec = NULL;
    node->files = NULL;
    return node;
}

struct node *node_and(void)
{
    struct node *node = malloc(sizeof(struct node));
    node->expr_type = OPERATOR;
    node->type = AND;
    node->left = NULL;
    node->right = NULL;
    node->value = "-a";
    node->exec = NULL;
    node->files = NULL;
    return node;
}

static struct node *node_not(struct node **nodes, int i, int len)
{
    if (i >= len)
    {
        free_nodes(nodes);
        errx(1, "Syntax Error: Missing operand after '!'.");
    }

    struct node *node = malloc(sizeof(struct node));
    node->expr_type = OPERATOR;
    node->type = NOT;
    node->left = NULL;
    node->right = NULL;
    node->value = "!";
    node->exec = NULL;
    node->files = NULL;
    return node;
}

static struct node *node_lpar(void)
{
    struct node *node = malloc(sizeof(struct node));
    node->expr_type = OPERATOR;
    node->type = LPAR;
    node->left = NULL;
    node->right = NULL;
    node->value = "(";
    node->exec = NULL;
    node->files = NULL;
    return node;
}

static struct node *node_rpar(void)
{
    struct node *node = malloc(sizeof(struct node));
    node->expr_type = OPERATOR;
    node->type = RPAR;
    node->left = NULL;
    node->right = NULL;
    node->value = ")";
    node->exec = NULL;
    node->files = NULL;
    return node;
}

void free_nodes(struct node **nodes)
{
    if (!nodes)
        return;

    for (int i = 0; nodes[i]; i++)
    {
        free(nodes[i]->exec);
        free(nodes[i]->files);
        free(nodes[i]);
    }
    free(nodes);
}

int get_nodes_len(struct node **nodes)
{
    int len = 0;
    for (int i = 0; nodes[i]; i++)
        len++;

    return len;
}

static struct node **add_node(struct node **nodes, struct node *node)
{
    int i = 0;
    while (nodes[i])
        i++;

    nodes[i] = node;
    return nodes;
}

struct node **init_nodes_list(int len)
{
    struct node **nodes = malloc(sizeof(struct node *) * (len + 1));
    for (int i = 0; i <= len; i++)
        nodes[i] = NULL;

    return nodes;
}

struct node **lexer(int len, char **argv)
{
    if (len == 0)
        return NULL;

    int i = 0;
    struct node **nodes = init_nodes_list(len);

    while (i < len)
    {
        if (strcmp(argv[i], "-print") == 0 || strcmp(argv[i], "-delete") == 0)
            nodes = add_node(nodes, node_printdel(argv[i]));
        else if (strcmp(argv[i], "-name") == 0)
            nodes = add_node(nodes, node_name(nodes, argv[++i]));
        else if (strcmp(argv[i], "-type") == 0)
            nodes = add_node(nodes, node_type(nodes, argv[++i]));
        else if (strcmp(argv[i], "-newer") == 0)
            nodes = add_node(nodes, node_newer(nodes, argv[++i]));
        else if (strcmp(argv[i], "-perm") == 0)
            nodes = add_node(nodes, node_perm(nodes, argv[++i]));
        else if (strcmp(argv[i], "-user") == 0)
            nodes = add_node(nodes, node_user(nodes, argv[++i]));
        else if (strcmp(argv[i], "-group") == 0)
            nodes = add_node(nodes, node_group(nodes, argv[++i]));
        else if (strcmp(argv[i], "-exec") == 0)
            nodes = add_node(nodes, handle_exec(nodes, argv, &i, len));
        else if (strcmp(argv[i], "-execdir") == 0)
            nodes = add_node(nodes, node_execdir(nodes, argv, &i, len));
        else if (strcmp(argv[i], "-o") == 0)
            nodes = add_node(nodes, node_or());
        else if (strcmp(argv[i], "-a") == 0)
            nodes = add_node(nodes, node_and());
        else if (strcmp(argv[i], "!") == 0)
            nodes = add_node(nodes, node_not(nodes, i + 1, len));
        else if (strcmp(argv[i], "(") == 0)
            nodes = add_node(nodes, node_lpar());
        else if (strcmp(argv[i], ")") == 0)
            nodes = add_node(nodes, node_rpar());
        else
        {
            free_nodes(nodes);
            errx(1, "Lexical Error: Invalid Token: %s", argv[i]);
        }

        i++;
    }

    nodes = realloc(nodes, sizeof(struct node *) * (get_nodes_len(nodes) + 1));
    return nodes;
}
