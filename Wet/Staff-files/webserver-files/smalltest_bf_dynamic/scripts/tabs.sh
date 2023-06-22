#!/bin/bash


#change ur output.cgi so it sleep for 10 secs ( sleep(10) )
cd ..

echo "dynamic or blockflush ? (choose option by number)"
select yn in "dynamic" "blockflush"; do
    case $yn in
        dynamic )     gnome-terminal --tab -- bash -c "./server 2002 4 2 dynamic 3; exec bash -i";   break;;
        blockflush )  gnome-terminal --tab -- bash -c "./server 2002 3 3 bf ; exec bash -i" ;    gnome-terminal --tab -- bash -c "./client localhost 2002 output.cgi?1; exec bash -i"; break;;
    esac
done


sleep 1


gnome-terminal --tab -- bash -c "./client localhost 2002 output.cgi?1; exec bash -i"
gnome-terminal --tab -- bash -c "./client localhost 2002 output.cgi?1; exec bash -i"


case $yn in
        dynamic )     gnome-terminal --tab --title="test tab" -- bash -c "./scripts/dynamic.sh;exec bash -i";   break;;
        blockflush )  gnome-terminal --tab --title="test tab" -- bash -c "./scripts/blockflush.sh;exec bash -i";    break;;
    esac




#this one for dynamic : run script then do ./client localhost 2002 home.html  twice , first 1 should close connection 2nd one should return output of servestatic.
#gnome-terminal --tab -- bash -c "./server 2002 5 3 dynamic 5; exec bash -i"

#this one for blockflush : run script then do ./client localhost 2002 home.html it should wait untill dynamic request finish (10 secs) then close connection
#gnome-terminal --tab -- bash -c "./server 2002 3 3 bf ; exec bash -i"

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
