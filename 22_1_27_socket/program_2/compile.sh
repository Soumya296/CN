for i in *.c
do
    gcc -w -g3 -o3 $i -o ${i%.c} -pthread
done