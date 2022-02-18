#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

void *read_thread(void *arg)
{
    while (1)
    {
        int rffd = open("client3", O_RDONLY);
        char buf[1024];
        int sz = read(rffd, buf, 1024);
        if (sz > 0)
            printf("\nClient 3 reads the message: %s\n", buf);
        close(rffd);
    }
}

void *write_thread(void *arg)
{
    while (1)
    {
        int wffd = open("server", O_WRONLY);
        char buf[1024];
        scanf("%s", buf);
        char buf1[] = "C3: ";
        strcat(buf1, buf);
        write(wffd, buf1, strlen(buf1));
        close(wffd);
    }
}

int main()
{
    pthread_t read_th, write_th;
    mkfifo("server", 0666);
    mkfifo("client3", 0666);
    pthread_create(&read_th, NULL, read_thread, NULL);
    pthread_create(&write_th, NULL, write_thread, NULL);
    pthread_join(read_th, NULL);
    pthread_join(write_th, NULL);
}