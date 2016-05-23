CC=gcc
CFLAGS=-Wall -Wextra
LIBS=-lcriu
 
EXEC=run


prog: 1.o
	sudo criu service -d
	$(CC) $(CFLAGS) -o $(EXEC) 1.o $(LIBS)

1.o: 1.c
	$(CC) -c 1.c


exec: prog
	sudo ./$(EXEC)

test: prog
	echo "PLEASE remove this line and extend the 'test' target"


.PHONY: clean

clean:
	-rm *.o $(EXEC) *.socket
	-rm -rf dump/
