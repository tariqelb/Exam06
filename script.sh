#!/bin/sh

i=1024
j=3024

while [ $i -lt $j ]
do
    cat ./script.sh | nc 127.0.0.1 8888 
    i=$((i + 1))
done
