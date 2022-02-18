#!/usr/bin/env bash

for i in *.c
do
    gcc -pthread -lrt -g3 -o3 $i -o ${i%.c}
done
