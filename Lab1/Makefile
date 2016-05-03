CC=gcc
CFLAGS=--std=c99 -Wall -Werror -O1
EXEC=run

prog: 1_2.o
	$(CC) $(CFLAGS) -o $(EXEC) 1_2.o

1_2.o: 1_2.c
	$(CC) -c 1_2.c

exec: prog
	./$(EXEC)

test: prog
	./test.sh

.PHONY: clean

clean:
	echo "clean starts"
	-rm *.o $(EXEC) log.txt
	echo "clean finished"