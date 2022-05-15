#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

int main(void)
{
    char *file_name = "temp.txt";

    int fd1 = open(file_name, O_RDWR);
    int fd2 = open(file_name, O_RDWR);
    
    for (char c = 'a'; c <= 'z'; c++)
        write(c % 2 ? fd1 : fd2, &c, 1);
    
    close(fd1);
    close(fd2);
    return 0;
}