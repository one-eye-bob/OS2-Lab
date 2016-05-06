#!/bin/bash
success=0
failure=0
NOW="$(date)"
echo Starting test run at $NOW >> log.txt
for i in `seq 1 5`
do
	make exec
	if [ 0 -eq $? ]
	then
		success=$((1 + success))
	else
		failure=$((1+ failure))	
	fi

done
echo "success: "$success", failure: "$failure>> log.txt
