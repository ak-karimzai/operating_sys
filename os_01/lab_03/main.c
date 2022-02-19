#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int fid = fork();

    if (fid == 0) {
        while (1)
            printf("%d ", getpid());
        exit(0);
    }
    else if (fid > 0) {
        while (1)
            printf("%d ", getpid());
        exit(0);
    }
    else
        puts("process creation failed!");
    return 1;
}