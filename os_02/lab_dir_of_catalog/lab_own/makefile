
ftw : main.o stack_t.o
	gcc -o $@ $^

stack_t.o : stack_t.c
	gcc -c -o $@ $^

main.o : main.c
	gcc -c -o $@ $^

.PHONY : clean

clean :
	rm -rf *.o *.out *.exe