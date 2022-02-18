#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main()
{
    int sz;
    char buf[100];

    sz = read(0, buf, 100);
    printf("%d\n", sz);
}