#!/bin/bash

CNT=10000
EXEC=./Debug_x86/Test/Test.exe

for ((n=0; n < $CNT; n++)); do
    OUT=`$EXEC  --rng-seed $RANDOM`
    if [[ $? -ne 0 ]]; then
        echo "$OUT"
    fi
    x=$(($n % 10))
    if [[ $x == 0 ]]; then
       echo $n
    fi
done
