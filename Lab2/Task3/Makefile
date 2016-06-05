CC=gcc
CFLAGS=-Wall -Wextra
LIBS=-lcriu

EXEC=run

TASK=3

prog: $(TASK).o
	$(CC) $(CFLAGS) -o $(EXEC) $(TASK).o $(LIBS)

$(TASK).o: $(TASK).c
	$(CC) -c $(TASK).c


exec: prog
	sudo ./$(EXEC) text.txt 3 #checkpoints every 3 sec

test: prog
	echo "PLEASE remove this line and extend the 'test' target"


.PHONY: clean

clean:
	# Delete everything except c files and Makefile
	sudo find . ! -name *.c ! -name Question3.1 ! -name Question3.2 ! -name Makefile ! -name Task2.3 -delete
