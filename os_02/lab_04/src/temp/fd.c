#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

void read_fd(void) {
    puts("fd: ");
    struct dirent *dirp;
    DIR *dp;
    char str[BUFSIZ];
    char path[BUFSIZ];
    dp = opendir("/proc/self/fd");
    while (dirp = readdir(dp)) {
        if (strcmp(dirp->d_name, ".") != 0 &&
            strcmp(dirp->d_name, "..") != 0) 
        {
            sprintf(path, "%s%s", "/proc/self/fd/", dirp->d_name);
            readlink(path, str, BUFSIZ);
            printf("%s -> %s\n", dirp->d_name, str);
        }
    }
    closedir(dp);
}

int main(void)
{
    // read_fd();
    execl("/bin/ls", "ls", "/proc/self/fd", NULL);
}