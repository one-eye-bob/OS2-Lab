#!/bin/bash
NOW="$(date)"
echo Starting test run at $NOW >> log.txt
make
./run -n 10 -f 50&
for i in `seq 1 100`
do
	var=$RANDOM
	var="$[ $var % 100 ]"
	if [ $var -ge 60 ]
	then
		touch "requests/fail$i"
	else
		touch "requests/$i"
	fi
done
