CC=gcc
CFLAGS=-Wall -Wextra
LIBS=-lcriu

EXEC=run

TASK=2

prog: $(TASK).o
	sudo criu service -d
	$(CC) $(CFLAGS) -o $(EXEC) $(TASK).o $(LIBS)

$(TASK).o: $(TASK).c
	$(CC) -c $(TASK).c


exec: prog
	sudo ./$(EXEC) test.txt 3 #checkpoints every 3 sec

test: prog
	echo "PLEASE remove this line and extend the 'test' target"


.PHONY: clean

clean:
	# Delete everything except c files and Makefile
	sudo find . ! -name *.c ! -name Makefile ! -name Task2.3 -delete
