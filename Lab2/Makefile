CC=gcc
CFLAGS=-D_BSD_SOURCE --std=c99 -Wall -Werror -O1
EXEC=run

prog: 2_1.o
	$(CC) $(CFLAGS) -o $(EXEC) 2_1.o

2_1.o: 2_1.c
	$(CC) -c 2_1.c

exec: prog
	./$(EXEC)

test: prog
	./test.sh

.PHONY: clean

clean:
	echo "clean starts"
	-rm *.o $(EXEC) log.txt
	echo "clean finished"
