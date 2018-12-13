#!/bin/bash
for (( i=1; i <= 1000; i++ ))
do
    output="$(
      ./cmake-build-debug/src/tests/dummy_test
    )"

   if echo $output | grep -q "correct? 1";then
     echo $i OK
   else
     echo !!!!ERROR
   fi
done
