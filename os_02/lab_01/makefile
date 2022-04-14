daemon.exe : daemonise.o err.o
	gcc $^ -o $@ -lpthread

%.o : %.c
	gcc -c $^ -o $@

clean :
	rm -rf *.o *.exe *.out