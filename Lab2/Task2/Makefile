CC=gcc
CFLAGS=-Wall -Wextra
LIBS=-lcriu

EXEC=run
TEST=test.sh

TASK=2

prog: $(TASK).o
	sudo criu service -d
	$(CC) $(CFLAGS) -o $(EXEC) $(TASK).o $(LIBS)
	#Usage command: sudo ./run (output filename) (optional checkpoints interval in seconds)

$(TASK).o: $(TASK).c
	$(CC) -c $(TASK).c


exec: prog
	sudo ./$(EXEC) text.txt 3 #checkpoints every 3 sec

test: prog
	EXEC=$(EXEC) opfn=test.txt cpi=3 ./$(TEST)


.PHONY: clean

clean:
	# Delete everything except c files, the test script and Makefile
	sudo find . ! -name *.c ! -name Makefile ! -name $(TEST) -delete
