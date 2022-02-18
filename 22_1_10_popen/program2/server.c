#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

void *write_to_server(void *arg)
{
    while (1)
    {
        printf("\nInput in Server\n");
        char buf[1024];
        scanf("%s", buf);

        int wffd = open("server", O_WRONLY);
        write(wffd, buf, strlen(buf));
        close(wffd);
    }
}

void *write_to_client(void *arg)
{
    while (1)
    {
        char buf[1024];
        int rffd = open("server", O_RDONLY);
        int sz = read(rffd, buf, 1024);
        close(rffd);

        if (sz > 0)
        {
            char buf1[] = "from Server : ";
            int ffd1 = open("client1", O_WRONLY);
            int ffd2 = open("client2", O_WRONLY);
            int ffd3 = open("client3", O_WRONLY);

            strcat(buf1, buf);
            write(ffd1, buf1, strlen(buf1));
            write(ffd2, buf1, strlen(buf1));
            write(ffd3, buf1, strlen(buf1));

            close(ffd1);
            close(ffd2);
            close(ffd3);
        }
    }
}

int main()
{
    pthread_t t1, t2;
    mkfifo("server", 0666);
    mkfifo("client1", 0666);
    mkfifo("client2", 0666);
    mkfifo("client3", 0666);

    pthread_create(&t1, NULL, write_to_server, NULL);
    pthread_create(&t2, NULL, write_to_client, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
}