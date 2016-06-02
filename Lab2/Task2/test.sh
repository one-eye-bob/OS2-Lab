#!/bin/bash
#Arguments of this shell
#	EXEC: the name of execution file
#	opfn: the filename of output (the reguired first argument of EXEC)
#	cpi: the checkpoints interval in seconds

echo TEST# Starting

#RUN the program
#sudo ./$EXEC $opfn $cpi 

#get the process ID of the worker; to compare it later with the ID of the restored process after crash
workerpid=$! # (not working) This should get the pid of the worker


#enter random "small" positve integers n1, n2, n3 (TODO) to stdin, one after the other
n1=$((RANDOM % 10))
n2=$((RANDOM % 10))
n3=$((RANDOM % 10))

#two possibilities:
#	kill the worker process by external signal (such as SIGKILL)
#	kill $workerpid
#OR
#	enter a random negative number (TODO) to stdin; to make the woker crashes
	n4=$((-RANDOM))
#	check if this woker process exited with a value equals to this negative number
#	$?==$n4

#TEST-1: check if a process with the ID the previus worker is running
#$workerpid==$!

#TEST-2:
#enter a non-number char(s); to end the program and write all generated number to the output file
#check if the data of the generated numbers is not lost by:
#	(1) comparing the number of lines equals n1+n2+n3+1 (the last return char)
#	(2) each number must be not bigger than their range, whether n1, n2 or n3

echo TEST# The end


