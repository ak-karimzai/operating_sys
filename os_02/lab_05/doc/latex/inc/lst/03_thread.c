#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define BUF_N 20
#define THD_N 2

struct thread_data {
    int thd_n;
    char *file_name;
};

void *open_read(void *args)
{
    struct thread_data *tdata = (struct thread_data *)args;
    int fd = open(tdata->file_name, O_RDWR);

    for (char c = 'a' + tdata->thd_n; c <= 'z'; c += (tdata->thd_n + 1))
        write(fd, &c, 1);
}

int main(void)
{
    char *file_name = "temp.txt";

    pthread_t tid[THD_N];
    for (size_t i = 0; i < THD_N; i++)
    {
        struct thread_data tdata = { i, file_name };
        pthread_create((tid + i), NULL, open_read, (void *)&tdata);
    }
    
    for (size_t i = 0; i < THD_N; i++)
        pthread_join(tid[i], NULL);
    return 0;
}