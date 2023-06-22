#!/bin/bash


#change ur output.cgi so it sleep for 10 secs ( sleep(10) )
> errors.txt
> prevstate.txt
> empty.txt
>expected.out

./client localhost 2002 home.html  > trash.out 2> errors.txt #should get error and size changes by 1 (goes up to 3 now)
sleep 0.5
echo "Rio_readlineb error: Connection reset by peer" >expected.out

if (!(cmp -s errors.txt  expected.out  ) ); then

    if(!(cmp -s errors.txt prevstate.txt) || !( cmp -s trash.out empty.txt)); then
        echo "failed"
        echo "1"
        exec bash -i
    fi
fi

./client localhost 2002 home.html  > trash.out 2> errors.txt #this should work
sleep 0.5
if (cmp -s trash.out empty.txt) ; then
    if(!(cmp -s errors.txt  empty.txt  )); then
        echo "failed"
        echo "2"
        exec bash -i
    fi
fi

sleep 0.1
cat errors.txt > prevstate.txt
sleep 0.1

gnome-terminal --tab -- bash -c "./client localhost 2002 output.cgi?1; exec bash -i"
sleep 1

./client localhost 2002 home.html > trash.out  2> errors.txt #should get error 
echo "Rio_readlineb error: Connection reset by peer" >expected.out
if (!(cmp -s errors.txt  expected.out  )); then
    if(!(cmp -s errors.txt prevstate.txt)|| !(cmp -s trash.out empty.txt)); then
        echo "failed"
        echo "3"
        exec bash -i
    fi
fi


sleep 0.1

cat errors.txt > prevstate.txt
sleep 0.1



./client localhost 2002 home.html > trash.out  2>> errors.txt #fail
sleep 0.5

echo "Rio_readlineb error: Connection reset by peer" >>expected.out
if (!(cmp -s errors.txt  expected.out) ); then
    if(!(cmp -s errors.txt prevstate.txt) || !(cmp -s trash.out empty.txt)); then
        echo "failed"
        echo "4"
        exec bash -i
    fi
fi

sleep 0.1

cat errors.txt > prevstate.txt
sleep 0.1
./client localhost 2002 home.html > trash.out  2>> errors.txt   #fail


sleep 1
echo "Rio_readlineb error: Connection reset by peer" >>expected.out
if (!(cmp -s errors.txt  expected.out  )); then
    if(!(cmp -s errors.txt prevstate.txt)|| !(cmp -s trash.out empty.txt)); then
        echo "failed"
        echo "5"
        exec bash -i
    fi
fi
sleep 0.1

cat errors.txt > prevstate.txt
sleep 0.1



if (cmp -s errors.txt  expected.out  ) ; then
echo "SUCCESS"
echo "remember to change output.c back to original before submitting"
else
    if(!(cmp -s errors.txt prevstate.txt) || !(cmp -s trash.out empty.txt)); then
            echo "failed"
            echo "end"
            exec bash -i
    fi
echo "SUCCESS"
echo ""

echo "remember to change output.c back to original before submitting"

fi
rm -f empty.txt
rm -f errors.txt
rm -f trash.out 
echo "u can close this tab"
while (true ); do true; done
#everything below here is irrelevant.
#gnome-terminal -- bash -c --tab --command="./server 2002 5 3 dynamic 5 ; sleep 10" --tab --command="./runmultipledyn.sh" 
#gnome-terminal --tab --command="./server 2002 5 3 dynamic 5 ; sleep 10" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh"

#this one for blockflush : run script then do ./client localhost 2002 home.html it should wait untill dynamic request finish (10 secs) then close connection
#gnome-terminal --tab --command="./server 2002 3 3 bf" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh"



# --tab --title="stat1" --command="./runmultiplestat_error.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --title="stat1" --command="./runmultiplestat_error.sh" --tab --command="./runmultipledyn.sh" #--tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" --tab --command="./runmultipledyn.sh" 
#gnome-terminal --tab --title="dyn1" --command="./runmultipledyn.sh"  --tab --title="stat1" --command="./runmultiplestat_error.sh" --tab --title="dyn2" --command="./runmultipledyn.sh"  --tab --title="stat2" --command="./runmultiplestat_error.sh"  --tab --title="dyn1" --command="./runmultipledyn.sh"  --tab --title="stat1" --command="./runmultiplestat_error.sh"--tab --title="dyn1" --command="./runmultipledyn.sh"  --tab --title="stat1" --command="./runmultiplestat_error.sh"--tab --title="dyn1" --command="./runmultipledyn.sh"  --tab --title="stat1" --command="./runmultiplestat_error.sh"--tab --title="dyn1" --command="./runmultipledyn.sh"  --tab --title="stat1" --command="./runmultiplestat_error.sh"
#gnome-terminal --tab -e ./runmultipledyn.sh --tab -e ./runmultipledyn.sh 

#gnome-terminal -- ./runmultipledyn.sh
#gnome-terminal -- ./runmultiplestat_error.sh
#gnome-terminal -- ./runmultipledyn.sh
