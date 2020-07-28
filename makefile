all: Trinity

console.o: console.h console.c
	gcc -c console.c -Wall -Wextra

cli.o: cli.h cli.c
	gcc -c cli.c -Wall -Wextra

file.o: file.h file.c
	gcc -c file.c -Wall -Wextra

main.o: console.h cli.h file.h main.c
	gcc -c main.c

Trinity: console.o cli.o file.o main.o
	gcc console.o cli.o file.o main.o -o Trinity -lpthread

clean:
	rm -f *.o Trinity
