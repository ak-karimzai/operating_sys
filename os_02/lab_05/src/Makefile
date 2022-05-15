cc := gcc 
cflags := -Werror -Wall

all : 01.exe 02.exe 03.exe 01_thread.exe 02_thread.exe 03_thread.exe  

01.exe : 01.c
	$(cc) -o $@ $^

02.exe : 02.c
	$(cc) -o $@ $^

03.exe : 03.c
	$(cc) -o $@ $^

01_thread.exe : 01_thread.c
	$(cc) -o $@ $^

02_thread.exe : 02_thread.c
	$(cc) -o $@ $^

03_thread.exe : 03_thread.c
	$(cc) -o $@ $^

clean : 
	rm -rf *.exe *.out