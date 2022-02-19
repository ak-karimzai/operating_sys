#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#define REP_N 2 // loop rep time
#define SLP_T 2 // sleep time

enum error_code
{
    ok,
    error
};

int main()
{
    int child[REP_N];

    printf("Parent process PID = %d, Group: %d\n", getpid(), getpgrp());

    for (size_t i = 0; i < REP_N; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("Can't fork!");
            return error;
        } else if (!pid) {
            sleep(SLP_T);
            printf("\nChild process PID = %d, PPID = %d, , Group: %d\n", \
                                    getpid(), getppid(), getpgrp());
            return ok;
        } else {
            child[i] = pid;
        }

    }
    puts("msg from parent process");
    
    for (size_t i = 0; i < REP_N; i++)
        printf("child[%zu].pid = %d\n", i, child[i]);
    
    return ok;
}