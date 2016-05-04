#!/bin/bash
success=0
failure=0
NOW="$(date)"
echo Starting test run at $NOW >> log.txt
for i in `seq 1 5`
do
	make 
	#generate random MAX_FORKS in [0,100[ for testing
	./run -n $((RANDOM % 100))
	if [ 0 -eq $? ]
	then
		success=$((1 + success))
	else
		failure=$((1+ failure))	
	fi

done
echo "success: "$success", failure: "$failure>> log.txt
