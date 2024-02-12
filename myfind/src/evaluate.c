#include "evaluate.h"

#define FUNS_LEN 11

static char *get_filename(char *path)
{
    char *saveptr;
    char *str = malloc(sizeof(char) * (strlen(path) + 1));
    str = strcpy(str, path);
    char *token = strtok_r(str, "/", &saveptr);
    char *res = malloc(sizeof(char) * (strlen(token) + 1));

    while (token)
    {
        res = realloc(res, sizeof(char) * (strlen(token) + 1));
        res = strcpy(res, token);
        token = strtok_r(NULL, "/", &saveptr);
    }

    free(str);
    return res;
}

static bool print(char *file, char *value, int *err)
{
    if (*err)
        return false;

    if (strcmp(value, "-print") == 0)
        printf("%s\n", file);

    return true;
}

static bool delete_file(char *file, char *value, int *err)
{
    if (strcmp(value, "-delete") == 0 && remove(file) == -1)
    {
        *err = 1;
        return false;
    }

    return true;
}

static bool name(char *file, char *value, int *err)
{
    if (*err)
        return false;

    char *name = get_filename(file);
    int match = fnmatch(value, name, 0);

    free(name);
    if (match == 0)
        return true;

    return false;
}

static bool validate_type(char *option, struct stat buf, bool seen_opt[7],
                          bool *res)
{
    if (option[0] == 'b' && !seen_opt[0])
    {
        *res |= S_ISBLK(buf.st_mode);
        seen_opt[0] = true;
    }
    else if (option[0] == 'c' && !seen_opt[1])
    {
        *res |= S_ISCHR(buf.st_mode);
        seen_opt[1] = true;
    }
    else if (option[0] == 'd' && !seen_opt[2])
    {
        *res |= S_ISDIR(buf.st_mode);
        seen_opt[2] = true;
    }
    else if (option[0] == 'f' && !seen_opt[3])
    {
        *res |= S_ISREG(buf.st_mode);
        seen_opt[3] = true;
    }
    else if (option[0] == 'l' && !seen_opt[4])
    {
        *res |= S_ISLNK(buf.st_mode);
        seen_opt[4] = true;
    }
    else if (option[0] == 'p' && !seen_opt[5])
    {
        *res |= S_ISFIFO(buf.st_mode);
        seen_opt[5] = true;
    }
    else if (option[0] == 's' && !seen_opt[6])
    {
        *res |= S_ISSOCK(buf.st_mode);
        seen_opt[6] = true;
    }
    else
        return false;

    return true;
}

static bool type(char *file, char *value, int *err)
{
    struct stat buf;
    int s = lstat(file, &buf);
    if (s == -1 || value[0] == ',' || value[strlen(value) - 1] == ',')
    {
        *err = 1;
        return false;
    }

    for (size_t i = 0; i < strlen(value) - 1; i++)
    {
        if (value[i] == ',' && value[i + 1] == ',')
        {
            *err = 1;
            return false;
        }
    }

    char *saveptr;
    char *val = malloc(sizeof(char) * (strlen(value) + 1));
    val = strcpy(val, value);
    char *option = strtok_r(val, ",", &saveptr);

    bool res = false;
    bool seen_opt[7] = { false };
    while (option)
    {
        if (strlen(option) != 1 || !validate_type(option, buf, seen_opt, &res))
        {
            free(val);
            *err = 1;
            return false;
        }

        option = strtok_r(NULL, ",", &saveptr);
    }

    free(val);
    return res;
}

static bool newer(char *file, char *value, int option, int *err)
{
    struct stat buf1;
    struct stat buf2;

    int s1;
    int s2;
    if (option != 2)
    {
        s1 = stat(file, &buf1);
        s2 = stat(value, &buf2);
        if (s1 == -1)
        {
            if (lstat(file, &buf1) == -1)
            {
                *err = 1;
                return false;
            }
        }

        if (s2 == -1)
        {
            if (lstat(value, &buf2) == -1)
            {
                *err = 1;
                return false;
            }
        }
    }
    else
    {
        s1 = lstat(file, &buf1);
        s2 = lstat(value, &buf2);
        if (s1 == -1 || s2 == -1)
        {
            *err = 1;
            return false;
        }
    }

    if (buf1.st_mtim.tv_sec == buf2.st_mtim.tv_sec)
        return buf1.st_mtim.tv_nsec > buf2.st_mtim.tv_nsec;

    return buf1.st_mtim.tv_sec > buf2.st_mtim.tv_sec;
}

static int to_oct(mode_t mode)
{
    char buf[7];
    char val[4];
    snprintf(buf, 8, "%o", mode);

    size_t j = 0;
    for (size_t i = strlen(buf) - 3; i < strlen(buf); i++)
    {
        val[j] = buf[i];
        j++;
    }

    val[j] = '\0';
    return atoi(val);
}

static char *binary(int n, int *i, char *bin)
{
    int count = 0;
    while (count < 3)
    {
        bin[*i] = (n % 2) + '0';
        n /= 2;
        (*i)++;
        count++;
    }

    char tmp = bin[*i - 3];
    bin[*i - 3] = bin[*i - 1];
    bin[*i - 1] = tmp;

    return bin;
}

static char *to_bin(int mode)
{
    char *bin = malloc(sizeof(char) * 10);

    int n1 = mode / 100;
    int n2 = (mode % 100) / 10;
    int n3 = (mode % 10);

    int i = 0;
    binary(n1, &i, bin);
    binary(n2, &i, bin);
    binary(n3, &i, bin);

    bin[9] = '\0';
    return bin;
}

static bool check_dash(char *b1, char *b2)
{
    for (int i = 0; b1[i] != '\0'; i++)
    {
        if (b1[i] == '1' && b2[i] != '1')
            return false;
    }

    return true;
}

static bool check_slash(char *b1, char *b2)
{
    for (int i = 0; b2[i] != '\0'; i++)
    {
        if (b2[i] == '1' && b1[i] == '1')
            return true;
    }

    return false;
}

static bool is_num(char *val)
{
    if (strlen(val) > 4)
        return false;

    for (int i = 0; val[i] != '\0'; i++)
    {
        if (!isdigit(val[i]) || (val[i] - '0') >= 8)
            return false;
    }

    return true;
}

static bool perm(char *file, char *value, int *err)
{
    struct stat buf;
    int s = lstat(file, &buf);
    if (s == -1)
    {
        *err = 1;
        return false;
    }

    if (value[0] == '-')
    {
        if (!is_num(value + 1))
        {
            *err = 1;
            return false;
        }

        char *bin1 = to_bin(atoi(value + 1));
        char *bin2 = to_bin(to_oct(buf.st_mode));
        bool res = check_dash(bin1, bin2);

        free(bin1);
        free(bin2);
        return res;
    }
    else if (value[0] == '/')
    {
        if (!is_num(value + 1))
        {
            *err = 1;
            return false;
        }

        char *bin1 = to_bin(atoi(value + 1));
        char *bin2 = to_bin(to_oct(buf.st_mode));

        bool res;
        if (strcmp(bin1, "000000000") == 0)
            res = check_dash(bin1, bin2);
        else
            res = check_slash(bin1, bin2);

        free(bin1);
        free(bin2);
        return res;
    }
    else
    {
        if (!is_num(value))
        {
            *err = 1;
            return false;
        }

        int mode = atoi(value);
        int oct = to_oct(buf.st_mode);
        return mode == oct;
    }

    return true;
}

static bool user(char *file, char *value, int *err)
{
    struct stat buf;
    int s = lstat(file, &buf);
    if (s == -1)
    {
        *err = 1;
        return false;
    }

    struct passwd *pw = getpwnam(value);
    if (!pw)
        errx(1, "User '%s' was not found.", value);

    return pw->pw_uid == buf.st_uid;
}

static bool group(char *file, char *value, int *err)
{
    struct stat buf;
    int s = lstat(file, &buf);
    if (s == -1)
    {
        *err = 1;
        return false;
    }

    struct group *gr = getgrnam(value);
    if (!gr)
        errx(1, "Group '%s' was not found.", value);

    return gr->gr_gid == buf.st_gid;
}

static char *replace(char *str, char *file, size_t len, size_t i)
{
    char *res = malloc(sizeof(char) * (len + 1));

    size_t j = 0;
    while (j < i)
    {
        res[j] = str[j];
        j++;
    }

    size_t k = 0;
    while (k < strlen(file))
    {
        res[j] = file[k];
        j++;
        k++;
    }

    i += 2;
    while (j < len)
    {
        res[j] = str[i];
        i++;
        j++;
    }

    res[len] = '\0';
    return res;
}

static char *replace_brackets(char *str, char *file)
{
    size_t len = strlen(str);

    size_t i = 0;
    size_t n = 0;
    while (i < strlen(str))
    {
        if (i + 1 < strlen(str) && str[i] == '{' && str[i + 1] == '}')
        {
            len += strlen(file) - 2;
            char *tmp = str;
            str = replace(str, file, len, i);

            if (n > 0)
                free(tmp);
            n++;
        }

        i++;
    }

    return str;
}

static void free_exec(char **copy, char **exec)
{
    for (int i = 0; copy[i]; i++)
    {
        if (strcmp(copy[i], exec[i]) != 0)
            free(copy[i]);
    }

    free(copy);
}

static bool exec(char *file, char **exec, int *err)
{
    char **new_exec = copy_exec(exec);
    for (int i = 0; new_exec[i]; i++)
        new_exec[i] = replace_brackets(new_exec[i], file);

    pid_t pid = fork();
    if (pid == -1)
    {
        free_exec(new_exec, exec);
        *err = 1;
        return false;
    }
    if (pid == 0)
    {
        execvp(new_exec[0], new_exec);
        free_exec(new_exec, exec);
        *err = 1;
        return false;
    }
    else
    {
        int status = 0;
        waitpid(pid, &status, 0);
        free_exec(new_exec, exec);
        return !status;
    }

    free_exec(new_exec, exec);
    return true;
}

static bool check_execplus(char *str)
{
    if (strcmp(str, "{}") == 0)
        return true;

    if (strstr(str, "{}"))
        return false;

    return true;
}

static bool execplus(char **files, char **exec, int *err)
{
    char **new_exec = copy_exec(exec);
    size_t count = 0;
    for (int i = 0; new_exec[i]; i++)
    {
        if (!check_execplus(new_exec[i]))
        {
            free(new_exec);
            *err = 1;
            return false;
        }
        else if (strcmp(new_exec[i], "{}") == 0)
        {
            if (count == 0 && i == get_files_len(new_exec) - 1)
            {
                new_exec[i] = files[0];
                for (int j = 1; j < get_files_len(files); j++)
                    new_exec = add_files(new_exec, files[j]);

                i += get_files_len(files) - 1;
                count++;
            }
            else
            {
                free(new_exec);
                *err = 1;
                return false;
            }
        }
    }

    if (count == 0)
    {
        free(new_exec);
        *err = 1;
        return false;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        free(new_exec);
        *err = 1;
        return false;
    }
    if (pid == 0)
    {
        execvp(new_exec[0], new_exec);
        free(new_exec);
        *err = 1;
        return false;
    }
    else
    {
        int status = 0;
        waitpid(pid, &status, 0);
        free(new_exec);
        return !status;
    }

    free(new_exec);
    return true;
}

static bool execplus_add(char *file, struct node *node)
{
    char *file_copy = malloc(sizeof(char) * (strlen(file) + 1));
    file_copy = strcpy(file_copy, file);
    node->files = add_files(node->files, file_copy);
    return true;
}

static bool execdir(char *file, char **exec, int *err)
{
    char *name = get_filename(file);
    char *filename = concat("./", name);

    char **new_exec = copy_exec(exec);
    for (int i = 0; new_exec[i]; i++)
        new_exec[i] = replace_brackets(new_exec[i], filename);

    pid_t pid = fork();
    if (pid == -1)
    {
        free(filename);
        free(name);
        free_exec(new_exec, exec);
        *err = 1;
        return false;
    }
    if (pid == 0)
    {
        int len = strlen(file) - strlen(name);
        if (len > 0)
        {
            file[len] = '\0';
            chdir(file);
        }

        execvp(new_exec[0], new_exec);
        free(filename);
        free(name);
        free_exec(new_exec, exec);
        *err = 1;
        return false;
    }
    else
    {
        int status = 0;
        waitpid(pid, &status, 0);

        free(filename);
        free(name);
        free_exec(new_exec, exec);
        return !status;
    }

    free(filename);
    free(name);
    free_exec(new_exec, exec);
    return true;
}

static struct function funs[FUNS_LEN] = {
    {
        .type = PRINT,
        .fun = print,
        .fun2 = NULL,
        .fun3 = NULL,
        .fun4 = NULL,
    },
    {
        .type = DELETE,
        .fun = delete_file,
        .fun2 = NULL,
        .fun3 = NULL,
        .fun4 = NULL,
    },
    {
        .type = NAME,
        .fun = name,
        .fun2 = NULL,
        .fun3 = NULL,
        .fun4 = NULL,
    },
    {
        .type = TYPE,
        .fun = type,
        .fun2 = NULL,
        .fun3 = NULL,
        .fun4 = NULL,
    },
    {
        .type = NEWER,
        .fun = NULL,
        .fun2 = NULL,
        .fun3 = newer,
        .fun4 = NULL,
    },
    {
        .type = PERM,
        .fun = perm,
        .fun2 = NULL,
        .fun3 = NULL,
        .fun4 = NULL,
    },
    {
        .type = USER,
        .fun = user,
        .fun2 = NULL,
        .fun3 = NULL,
        .fun4 = NULL,
    },
    {
        .type = GROUP,
        .fun = group,
        .fun2 = NULL,
        .fun3 = NULL,
        .fun4 = NULL,
    },
    {
        .type = EXEC,
        .fun = NULL,
        .fun2 = exec,
        .fun3 = NULL,
        .fun4 = NULL,
    },
    {
        .type = EXECPLUS,
        .fun = NULL,
        .fun2 = NULL,
        .fun3 = NULL,
        .fun4 = execplus_add,
    },
    {
        .type = EXECDIR,
        .fun = NULL,
        .fun2 = execdir,
        .fun3 = NULL,
        .fun4 = NULL,
    },
};

bool evaluate(char *file, struct node *ast, int *params[2])
{
    int *err = params[0];
    int option = *params[1];

    if (!ast)
        return true;

    if (ast->type == OR)
        return evaluate(file, ast->left, params)
            || evaluate(file, ast->right, params);
    if (ast->type == AND)
        return evaluate(file, ast->left, params)
            && evaluate(file, ast->right, params);
    if (ast->type == NOT)
        return !evaluate(file, ast->right, params);

    for (int i = 0; i < FUNS_LEN; i++)
    {
        if (funs[i].type == ast->type)
        {
            if (ast->type == EXECPLUS)
                return funs[i].fun4(file, ast);
            if (ast->type == EXEC || ast->type == EXECDIR)
                return funs[i].fun2(file, ast->exec, err);
            if (ast->type == NEWER)
                return funs[i].fun3(file, ast->value, option, err);

            return funs[i].fun(file, ast->value, err);
        }
    }

    return false;
}

void eval_execplus(struct node *ast, int *err)
{
    if (!ast)
        return;

    if (ast->type == EXECPLUS)
    {
        if (!ast->files || get_files_len(ast->files) == 0)
            return;

        execplus(ast->files, ast->exec, err);
    }

    eval_execplus(ast->left, err);
    eval_execplus(ast->right, err);
}
