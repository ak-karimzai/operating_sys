#include <stdio.h>

int main(int argc, char **argv) {
    char c;
    while ((c = getchar()) != '\n') {
        putc(c, stdout);
    }
    return 0;
}