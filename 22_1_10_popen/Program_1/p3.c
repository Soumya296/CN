#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
int main()
{
    int ffd;
    char *fifo = "red";
    mkfifo(fifo, 0666);
    ffd = open(fifo, O_WRONLY);
    while (1)
    {
        char buf[1024];
        scanf("%s", buf);
        int i;
        for (i = 0; buf[i] != '\0'; i++)
        {
            if (buf[i] >= 'a' && buf[i] <= 'z')
                buf[i] -= 32;
        }
        char buf1[] = " from P3";
        strcat(buf, buf1);
        printf("Message in p3 %s\n", buf);
        fflush(stdout);
        write(ffd, buf, strlen(buf) + 1);
    }
}