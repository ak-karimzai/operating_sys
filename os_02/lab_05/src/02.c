#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    int fd1 = open("alphs.txt", O_RDONLY);
    int fd2 = open("alphs.txt", O_RDONLY);

    int flags[2] = { 1, 1 };
    while (flags[0] == 1 && flags[1] == 1)
    {
        char c;
        flags[0] = read(fd1, &c, 1);
        if (flags[0] == 1) write(1, &c, 1);
        flags[1] = read(fd2, &c, 1);
        if (flags[1] == 1) write(1, &c, 1);
    }
    write(1, "\n", 1);
    return 0;
}