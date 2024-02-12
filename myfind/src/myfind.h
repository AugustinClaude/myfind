#ifndef MYFIND_H
#define MYFIND_H

#define _DEFAULT_SOURCE

#include <ctype.h>
#include <dirent.h>
#include <err.h>
#include <fnmatch.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum expr_type
{
    TEST,
    ACTION,
    OPERATOR,
};

enum type
{
    OR,
    AND,
    NOT,
    LPAR,
    RPAR,
    PRINT,
    DELETE,
    NAME,
    TYPE,
    NEWER,
    PERM,
    USER,
    GROUP,
    EXEC,
    EXECPLUS,
    EXECDIR,
};

struct node
{
    enum expr_type expr_type;
    enum type type;
    struct node *left;
    struct node *right;
    char *value;
    char **exec;
    char **files;
};

struct function
{
    enum type type;
    bool (*fun)(char *, char *, int *);
    bool (*fun2)(char *, char **, int *);
    bool (*fun3)(char *, char *, int, int *);
    bool (*fun4)(char *, struct node *);
};

char *concat(char *s1, char *s2);
int get_files_len(char **files);
char **add_files(char **files, char *file);

#endif /* ! MYFIND_H */
