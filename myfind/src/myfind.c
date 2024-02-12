#include "myfind.h"

#include "evaluate.h"
#include "lexer.h"
#include "shunting_yard.h"

char *concat(char *s1, char *s2)
{
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    size_t len = len1 + len2 + 2;
    char *newcur = malloc(sizeof(char) * len);

    size_t i;
    for (i = 0; i < len1; i++)
        newcur[i] = s1[i];
    for (size_t j = 0; j < len2; j++)
    {
        newcur[i] = s2[j];
        i++;
    }

    newcur[i] = '\0';
    return newcur;
}

static int get_cur(char *sp, char **cur)
{
    if (sp[strlen(sp) - 1] != '/')
    {
        *cur = concat(sp, "/");
        return 1;
    }

    *cur = sp;
    return 0;
}

static void free_files(char **files, int d)
{
    if (!files)
        return;

    if (!d)
    {
        for (int i = 1; i < get_files_len(files); i++)
            free(files[i]);
    }
    else
    {
        for (int i = 0; i < get_files_len(files) - 1; i++)
            free(files[i]);
    }

    free(files);
}

int get_files_len(char **files)
{
    int len = 0;
    for (int i = 0; files[i]; i++)
        len++;

    return len;
}

char **add_files(char **files, char *file)
{
    int len = get_files_len(files);
    files = realloc(files, sizeof(char *) * (len + 2));
    files[len] = file;
    files[len + 1] = NULL;
    return files;
}

static int *get_stat(char *sp, int *option, struct stat *buf, int *s)
{
    if (*option == 1)
    {
        *s = stat(sp, buf);
        if (*s == -1)
        {
            int s2 = lstat(sp, buf);
            if (s2 == -1)
            {
                fprintf(stderr, "myfind: '%s': No such file or directory.\n",
                        sp);
                return NULL;
            }
        }
    }
    else
    {
        *s = lstat(sp, buf);
        if (*s == -1)
        {
            fprintf(stderr, "myfind: '%s': No such file or directory.\n", sp);
            return NULL;
        }
    }

    return s;
}

static DIR *get_dir(char *sp, char ***files, int *first, int *params[3])
{
    int *d = params[1];
    int *option = params[2];

    struct stat buf;
    int s;
    if (!get_stat(sp, option, &buf, &s))
        return NULL;

    DIR *dir;
    if (sp[strlen(sp) - 1] != '/')
    {
        if (*first && !(*d))
            *files = add_files(*files, sp);

        if (S_ISLNK(buf.st_mode) && (*option == 2 || s == -1))
        {
            *first = -1;
            return NULL;
        }

        dir = opendir(sp);
        if (!dir)
        {
            if (!S_ISDIR(buf.st_mode))
                *first = -1;
            else
            {
                fprintf(stderr, "myfind: '%s': No such file or directory.\n",
                        sp);
                return NULL;
            }
        }
    }
    else
    {
        dir = opendir(sp);
        if (!dir)
        {
            fprintf(stderr, "myfind: '%s': No such file or directory.\n", sp);
            return NULL;
        }

        if (*first && !(*d))
            *files = add_files(*files, sp);
    }

    return dir;
}

void *error_file(int *params[3], int tofree, char *cur, char **files)
{
    int *err = params[0];
    int *d = params[1];
    *err = 1;
    fprintf(stderr, "myfind: Unable to open file.\n");
    if (tofree)
        free(cur);
    free_files(files, *d);
    return NULL;
}

static int *stat_file(char *newcur, int *option, struct stat *buf, int *s)
{
    if (*option == 1)
    {
        *s = stat(newcur, buf);
        if (*s == -1)
        {
            int s2 = lstat(newcur, buf);
            if (s2 == -1)
                return NULL;
        }
    }
    else
    {
        *s = lstat(newcur, buf);
        if (*s == -1)
            return NULL;
    }

    return s;
}

char **get_files(char *sp, char **files, int first, int *params[3])
{
    int *err = params[0];
    int *d = params[1];
    int *option = params[2];

    char *tmp = sp;
    DIR *dir = get_dir(sp, &files, &first, params);
    if (first == -1)
        return files;
    if (!dir)
    {
        *err = 1;
        free_files(files, *d);
        return NULL;
    }

    char *cur;
    int tofree = get_cur(sp, &cur);

    struct dirent *list = readdir(dir);
    while (list)
    {
        if (strcmp(list->d_name, ".") != 0 && strcmp(list->d_name, "..") != 0)
        {
            char *newcur = concat(cur, list->d_name);
            struct stat buf;
            int s;
            if (!stat_file(newcur, option, &buf, &s))
                return error_file(params, tofree, cur, files);

            if (!(*d))
                files = add_files(files, newcur);

            if (S_ISDIR(buf.st_mode)
                || (S_ISLNK(buf.st_mode) && *option == 1 && s != -1))
                files = get_files(newcur, files, 0, params);

            if (*d)
                files = add_files(files, newcur);
        }

        list = readdir(dir);
    }

    if (first && *d)
        files = add_files(files, tmp);

    if (tofree)
        free(cur);

    closedir(dir);
    return files;
}

static char *get_sp(char **begin, char **end)
{
    if (begin == end)
        return ".";
    return *begin;
}

static bool action(struct node *ast)
{
    if (!ast)
        return false;

    if (ast->expr_type == ACTION)
        return true;

    return action(ast->left) || action(ast->right);
}

static bool is_delete(struct node *ast)
{
    if (!ast)
        return false;

    if (ast->type == DELETE)
        return true;

    return is_delete(ast->left) || is_delete(ast->right);
}

static bool handle_options(char *opt, int *d, int *option)
{
    if (strcmp(opt, "-d") == 0)
    {
        *d = 1;
        return true;
    }
    if (strcmp(opt, "-H") == 0)
    {
        *option = 0;
        return true;
    }
    if (strcmp(opt, "-L") == 0)
    {
        *option = 1;
        return true;
    }
    if (strcmp(opt, "-P") == 0)
    {
        *option = 2;
        return true;
    }

    return false;
}

static int options(int argc, char **argv, int *i, int *option)
{
    int d = 0;
    while (*i < argc && argv[*i][0] == '-')
    {
        if (!handle_options(argv[*i], &d, option))
            break;

        (*i)++;
    }

    return d;
}

static void expressions(int argc, char **argv, int *i)
{
    while (*i < argc && argv[*i][0] != '-' && argv[*i][0] != '!'
           && argv[*i][0] != '(' && argv[*i][0] != ')')
        (*i)++;
}

static int **params_1(int *err, int *d, int *option)
{
    int **params = malloc(sizeof(int *) * 3);
    params[0] = err;
    params[1] = d;
    params[2] = option;

    return params;
}

static int **params_2(int *err, int *option)
{
    int **params = malloc(sizeof(int *) * 2);
    params[0] = err;
    params[1] = option;

    return params;
}

int main(int argc, char **argv)
{
    int i = 1;
    int option = 2;
    int d = options(argc, argv, &i, &option);

    char **begin = argv + i;
    expressions(argc, argv, &i);
    char **end = argv + i;

    struct node **nodes = lexer(argc - i, argv + i);
    struct node *ast = shunting_yard(nodes);

    if (is_delete(ast))
        d = 1;

    int err = 0;
    int **params = params_1(&err, &d, &option);

    int err_eval = 0;
    int **params2 = params_2(&err_eval, &option);

    bool res = true;
    do
    {
        char *sp = get_sp(begin, end);
        char **files = malloc(sizeof(char *));
        files[0] = NULL;
        files = get_files(sp, files, 1, params);

        if (files)
        {
            for (int i = 0; files[i]; i++)
            {
                res = evaluate(files[i], ast, params2);
                if (res && !action(ast))
                    printf("%s\n", files[i]);
            }

            free_files(files, d);
        }

        eval_execplus(ast, &err_eval);

        begin++;
    } while (begin < end);

    free(params);
    free(params2);

    free_ast(ast);
    free_nodes(nodes);

    if (err_eval)
        fprintf(stderr, "myfind: An error occured during evaluation.\n");
    return err || err_eval;
}
