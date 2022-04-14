#include <stdio.h>

int main(void)
{
    FILE *f = fopen("/proc/self/environ", "r");
    
    if (!f)
        return -1;
    size_t len;
    char buf[BUFSIZ];
    while ((len = fread(buf, 1, BUFSIZ, f)) > 0)
    {
        for (size_t i = 0; i < len; i++)
            if (buf[i] == 0)
                buf[i] = 10;
        
        if (len)
            buf[len - 1] = 0;
        printf("%s", buf);
    }
    fclose(f);
}