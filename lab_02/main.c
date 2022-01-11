#include <unistd.h>
#include <stdio.h>

int main(void) {
    int fid = fork();

    if (fid == 0)
        puts("Child process fid == 0");
    else if (fid > 0)
        printf("New process with id = %d\n", fid);
    else
        puts("process creation failed!");
    return 0;
}
