#!/bin/bash
for (( i=1; i <= 100; i++ ))
do
    output="$(
      ./cmake-build-debug/ts
    )"

   if echo $output | grep -q "correct? 1";then
     echo $i OK
   else
     echo !!!!ERROR
   fi
done
