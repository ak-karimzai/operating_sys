#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define REP_N 2 // loop rep time
#define SLP_T 2 // sleep time

enum error_code {
    ok,
    error
};

int main()
{
    int child[REP_N];
    char *const args[4] = { NULL };
    int pid;

    printf("Parent process PID = %d, Group: %d\n", getpid(), getpgrp());

    for (size_t i = 0; i < REP_N; i++) {
        pid = fork();

        if (pid == -1) {
            perror("Fork failed!");
            return error;
        } else if (pid == 0) {
            printf("\nChild process PID = %d, PPID = %d, Group: %d\n", \
                        getpid(), getppid(), getpgrp());
            
            int res;
            if (i == 0)
                res = execvp("/home/khalid/Desktop/c_labs/rk_03/a.out", args);
            else
                res = execvp("/home/khalid/Desktop/c_labs/rk_04/a.out", args);
            
            if (res == -1) {
                perror("exec failed!");
                return error;
            }
            
            return ok;
        } else {
            wait(NULL);
            child[i] = pid;
        }
    }

    puts("msg from parent process");
    
    for (size_t i = 0; i < REP_N; i++) {
        int status, ret_val = 0;
        
        pid_t child_pid = wait(&status);

        printf("\nChild process PID = %d, completed, status %d\n", \
                                    child_pid, status);
        
        if (WIFEXITED(ret_val)) {
            printf("Child process [№ = %zu] completed with %d exit code\n", 
                                        i + 1, WEXITSTATUS(ret_val));
        } else if (WIFSIGNALED(ret_val)) {
            printf("Child process [№ = %zu] completed with %d exit code\n", 
                                        i + 1, WTERMSIG(ret_val));
        } else if (WIFSTOPPED(ret_val)) {
            printf("Child process [№ = %zu] completed with %d exit code\n", 
                                        i + 1, WSTOPSIG(ret_val));
        }
    }
    
    puts("msg from parent process");
    
    for (size_t i = 0; i < REP_N; i++)
        printf("child[%zu].pid = %d\n", i, child[i]);
    
    return ok;
}