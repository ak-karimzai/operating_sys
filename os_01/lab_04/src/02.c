#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define REP_N 2 // loop rep time
#define SLP_T 2 // sleep time

enum error_code {
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
    
    for (size_t i = 0; i < REP_N; i++) {
        int status, ret_val = 0;
        
        pid_t child_pid = wait(&status);

        printf("\nChild process PID = %d, completed, status %d\n", \
                                    child_pid, status);
        
        if (WIFEXITED(ret_val))
            printf("Child process [№ = %zu] completed with %d exit code\n", 
                                        i + 1, WEXITSTATUS(ret_val));
        else if (WIFSIGNALED(ret_val))
            printf("Child process [№ = %zu] completed with %d exit code\n", 
                                        i + 1, WTERMSIG(ret_val));
        else if (WIFSTOPPED(ret_val))
            printf("Child process [№ = %zu] completed with %d exit code\n", 
                                        i + 1, WSTOPSIG(ret_val));
    }
    
    puts("msg from parent process");
    
    for (size_t i = 0; i < REP_N; i++)
        printf("child[%zu].pid = %d\n", i, child[i]);

    return ok;
}