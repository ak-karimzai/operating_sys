#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>

#define BUF_N 20
#define THD_N 2

void *open_read(void *args)
{
    int fd = *(int *)args;
    FILE *fs = fdopen(fd, "r");
    char buff[BUF_N];
    setvbuf(fs, buff, _IOFBF, BUF_N);

    char c;
    while (fscanf(fs, "%c", &c) == 1)
        fprintf(stdout, "%c", c);
}

int main(void)
{
    int fd = open("alphs.txt", O_RDONLY);

    pthread_t tid[THD_N];
    for (size_t i = 0; i < THD_N; i++)
        pthread_create((tid + i), NULL, open_read, (void *)&fd);
    
    for (size_t i = 0; i < THD_N; i++)
        pthread_join(tid[i], NULL);
    return 0;
}