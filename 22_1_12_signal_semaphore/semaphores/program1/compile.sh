#!/usr/bin/env bash

for i in *.c
do
    gcc -pthread -g3 -o3 $i -o ${i%.c}
done