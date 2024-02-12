#include <stdio.h>
#include <sys/stat.h>

void print_stat(struct stat *buf)
{
    printf("st_dev=%ld\n", buf->st_dev);
    printf("st_ino=%ld\n", buf->st_ino);
    printf("st_mode=%07o\n", buf->st_mode);
    printf("st_nlink=%ld\n", buf->st_nlink);
    printf("st_uid=%d\n", buf->st_uid);
    printf("st_gid=%d\n", buf->st_gid);
    printf("st_rdev=%ld\n", buf->st_rdev);
    printf("st_size=%ld\n", buf->st_size);
    printf("st_atime=%ld\n", buf->st_atime);
    printf("st_mtime=%ld\n", buf->st_mtime);
    printf("st_ctime=%ld\n", buf->st_ctime);
    printf("st_blksize=%ld\n", buf->st_blksize);
    printf("st_blocks=%ld\n", buf->st_blocks);
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return 0;

    for (int i = 1; i < argc; i++)
    {
        struct stat buf;
        int e = stat(argv[i], &buf);
        if (e == -1)
            return 1;

        print_stat(&buf);
    }

    return 0;
}
