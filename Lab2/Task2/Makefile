CC=gcc
CFLAGS=-Wall -Wextra
LIBS=-lcriu

EXEC=run

TASK=2

prog: $(TASK).o
	#sudo criu service -d
	$(CC) $(CFLAGS) -o $(EXEC) $(TASK).o #$(LIBS)

$(TASK).o: $(TASK).c
	$(CC) -c $(TASK).c


exec: prog
	#sudo ./$(EXEC)

test: prog
	./$(EXEC) test.txt


.PHONY: clean

clean:
	# Delete everything except c files and Makefile
	find . ! -name *.c ! -name Makefile -delete
