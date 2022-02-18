for i in *.c
do
    gcc  -g3 -o3 $i -o ${i%.c}
done