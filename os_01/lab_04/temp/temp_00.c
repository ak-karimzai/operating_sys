#include <unistd.h>


int main() {
    char *const arr[] = {"<", "text.txt", "c", NULL};

    execv("./02.exe", arr);
    return 0;
}