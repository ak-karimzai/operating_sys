CC := gcc
CFLAGS := -std=c11 -Wall -Werror


app.exe : main.o pagemap.o
	$(CC) $^ -o $@

main.o : main.c
	$(CC) $^ -c -o $@

pagemap.o : pagemap.c
	$(CC) $^ -c -o $@