#!/bin/bash

y=64

for(( i=0; $i<$y; ++i))
do
 
 echo $((RANDOM % 256))

done

exit 0
