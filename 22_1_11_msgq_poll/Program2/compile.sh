#!/usr/bin/env bash

for i in *.c
do
    gcc -w -g3 -o3 $i -o ${i%.c}
done
