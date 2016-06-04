#!/bin/bash
#Arguments of this shell
#	EXEC: the name of execution file
#	opfn: the filename of output (the reguired first argument of EXEC)
#	cpi: the checkpoints interval in seconds

#echo TEST# Starting

#RUN the program
#sudo ./$EXEC $opfn $cpis

#get the process ID of the worker; to compare it later with the ID of the restored process after crash
#workerpid=$! # (not working) This should get the pid of the worker
rounds=10
fr=35
counter=0
clen=0
while [ $counter -lt $rounds ]
do
	sleep 1
	f=$((RANDOM % 100))
	n=$((RANDOM % 10 + 1))
	if [ $f -le $fr ]; then
		n=$((0 - $n))
	fi
	sleep 2
	echo $n
	((counter++))
	clen=$(($clen + $n))
done

#enter a non-number char(s); to end the program and write all generated number to the output file
echo x

sleep 1


#TEST-1: check if a process with the ID the previus worker is running
#$workerpid==$!

#TEST-2:
#see if we have the desired output (legth check only)
content='cat $opfn'
alen=${#content}
if (($clen == $alen)); then
	echo "Test successful"
else
	echo "$opfn contains different numbers than should have been generated!"
fi

#kill processes
sudo pkill -f "./$EXEC $opfn $cpi"

#echo TEST# The end