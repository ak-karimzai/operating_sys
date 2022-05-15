#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define BUF_N 20
#define THD_N 2

void *open_read(void *args)
{
    int fd = open((char *)args, O_RDONLY);

    char c;
    while (read(fd, &c, 1) == 1)
        write(1, &c, 1);
}

int main(void)
{
    char *file_name = "alphs.txt";

    pthread_t tid[THD_N];
    for (size_t i = 0; i < THD_N; i++)
        pthread_create((tid + i), NULL, open_read, (void *)file_name);
    
    for (size_t i = 0; i < THD_N; i++)
        pthread_join(tid[i], NULL);
    return 0;
}