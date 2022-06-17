#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <fs/internal.h>

int main(void)
{
    int fd = open("temp", O_RDONLY);

    if (fd < 0) perror("Opening of file failed");
    else puts("file opened successfully");
    
    close(fd);
    return 0;
}