#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

void *write_in_pipe(void *pfd)
{
    fflush(stdout);
    int fd = *((int *)pfd);
    fflush(stdout);

    while (1)
    {
        char buf[1024];
        fflush(stdout);
        printf("\nEnter string to send from p1 to p2 \n");
        fflush(stdout);
        scanf("%s", buf);

        strcat(buf, " \0");
        write(fd, buf, strlen(buf));
    }
    return NULL;
}
void *read_from_fifo(void *ffd)
{
    int ffd1 = *((int *)ffd);
    while (1)
    {
        char buf[1024];
        read(ffd1, buf, 1024);
        printf("message from p3 to p1 : %s\n\n", buf);
        fflush(stdout);
    }
}
int main()
{
    pthread_t read_thread, write_thread;
    int fd = fileno(popen("./p2", "w"));
    char *fifo = "red";
    mkfifo(fifo, 0666);
    int ffd = open(fifo, O_RDONLY);
    pthread_create(&write_thread, NULL, write_in_pipe, &fd);
    pthread_create(&read_thread, NULL, read_from_fifo, &ffd);
    pthread_join(write_thread, NULL);
    pthread_join(read_thread, NULL);
}
