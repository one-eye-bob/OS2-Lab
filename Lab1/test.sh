#!/bin/bash
echo "testing 1_2"
POSVAL=0
NEGVAL=0
NOW="$(date)"
echo Starting test run at $NOW >> log.txt
for i in 1 2 3 4 5; do
	make exec
	RETVAL=$?
	[ $RETVAL -eq 0 ] && echo 'Success' >> log.txt && let POSVAL+=1
	[ $RETVAL -ne 0 ] && echo 'Failure' >> log.txt && let NEGVAL+=1
done

echo Finished test run... Counting $POSVAL successes and $NEGVAL failures >> log.txt