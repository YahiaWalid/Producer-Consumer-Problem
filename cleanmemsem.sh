#! /bin/bash

#check system semaphores/sharedmem/msgqueus
#ipcs -a | grep `whoami`

#cleans system v semaphores
ipcs -s | grep `whoami` | awk '{ print $2 }' | xargs -n1 ipcrm -s  2> /dev/null



: '
result1=$?
 
	if [ $result1 -eq 0 ]			
	then
		echo "result 1 is 0"
	else
		echo "result 1 is 1"		# shared mem was alr deleted before
	fi	
'



#cleans system v shared memory
ipcs -m | grep `whoami` | awk '{ print $2 }' | xargs -n1 ipcrm -m 2> /dev/null


: '
result2=$?

	if [ $result2 -eq 0 ]			
	then
		echo "result 2 is 0"
	fi
	
'

echo "cleaned SYSTEM V shared memory and semaphores"


	
