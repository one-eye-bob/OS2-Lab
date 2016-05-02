CC=gcc
CFLAGS=--std=c99 -Wall -Werror

prog: parent.o
	$(CC) $(CFLAGS) -o run parent.o

parent.o: parent.c
	$(CC) -c parent.c

exec: prog
	./run

test: prog
	./run
	./run
	./run
	./run
	./run

.PHONY: clean

clean:
	echo "clean starts"
	-rm *o run
	echo "clean finished"

