#!/bin/bash

SECONDS=0

> errors.txt
> empty.txt
>expected.out

./client localhost 2002 home.html > trash.out  2> errors.txt #should get error and size changes by 1;
echo "Rio_readlineb error: Connection reset by peer" >expected.out

while ( ! (cmp -s errors.txt  expected.out)) ; do 
    if(($SECONDS >12)) ;then 
        echo "u passed the allotted time and entered a infinite loop try again or check ur code or maybe u didnt change output.c with sleep 10 "
        echo ""
        exec bash -i

fi
done

echo "u closed connection after $SECONDS    (between 10 to 12 is the right amount)"

if ((($SECONDS<9)) || (($SECONDS >12)));then
   
            echo "failed"
            echo "u might've closed the connection before emptying the queue"
            exec bash -i
fi


./client localhost 2002 home.html > trash.out  2> errors.txt  #should be handled 
sleep 1


if(!(cmp -s errors.txt empty.txt) || (cmp -s trash.out empty.txt)) ; then 
  
    echo "failed"
    echo "u did not handle a static request with an empty queue"
    exec bash -i

fi

echo "SUCCESS"
echo ""

echo "remember to change output.c back to original before submitting"  


rm -f empty.txt
rm -f errors.txt
rm -f trash.out 
echo "u can close this tab"
while (true ); do true; done



