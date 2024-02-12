#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>

int myls(const char *path)
{
    DIR *dir = opendir(path);
    if (!dir)
    {
        fprintf(stderr, "simple_ls: %s: No such file or directory", path);
        return 1;
    }

    struct dirent *list = readdir(dir);
    while (list)
    {
        if (list->d_name[0] != '.')
            printf("%s\n", list->d_name);
        list = readdir(dir);
    }

    if (closedir(dir) == -1)
        return 1;

    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 1 || argc > 2)
    {
        fprintf(stderr, "Not enough arguments.");
        return 1;
    }

    char *path;
    if (argc == 1)
        path = ".";
    else
        path = argv[1];

    return myls(path);
}
