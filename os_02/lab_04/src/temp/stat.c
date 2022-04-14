#include <stdio.h>
#include <string.h>

int main(void)
{
    FILE *f = fopen("/proc/self/stat", "r");
    
    if (!f)
        return -1;
    char buf[BUFSIZ];
    fread(buf, 1, BUFSIZ, f);
    char *pch = strtok(buf, " ");

    puts("stat: ");
    while (pch)
    {
        printf("%s\n", pch);
        pch = strtok(NULL, " ");
    }
    fclose(f);
    return 0;
}