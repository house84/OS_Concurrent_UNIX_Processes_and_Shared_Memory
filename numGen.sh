#!/bin/bash

y=$(($RANDOM % 37 + 63))

echo $((y+1))

for(( i=0; $i<$y; ++i))
do
 
 echo $((RANDOM % 256))

done

exit 0
