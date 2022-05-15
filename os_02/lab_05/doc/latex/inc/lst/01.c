#include <stdio.h>
#include <fcntl.h>

#define BUF_N 20

int main(void)
{
    int fd = open("alphs.txt", O_RDONLY);

    FILE *fs1 = fdopen(fd, "r");
    FILE *fs2 = fdopen(fd, "r");
    char buff1[BUF_N], buff2[BUF_N];
    setvbuf(fs1, buff1, _IOFBF, BUF_N);
    setvbuf(fs2, buff2, _IOFBF, BUF_N);
    
    int flags[2] = { 1, 1 };
    while (flags[0] == 1 || flags[1] == 1)
    {
        char c[2];
        flags[0] = fscanf(fs1, "%c", &c[0]);
        flags[1] = fscanf(fs1, "%c", &c[1]);
        if (flags[0] == 1) fprintf(stdout, "%c", c[0]);
        if (flags[1] == 1) fprintf(stdout, "%c", c[1]);
    }
    fprintf(stdout, "\n");
    return 0;
}