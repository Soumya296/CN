#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<poll.h>

char * FIFO2 = "FIFO2";
char * FIFO3 = "FIFO3";
char * FIFO4 = "FIFO4";

int main()
{
    mkfifo(FIFO2, 0666);
    int ffd2 = open(FIFO2,O_RDONLY);

    mkfifo(FIFO3, 0666);
    int ffd3 = open(FIFO3,O_RDONLY);

    mkfifo(FIFO4, 0666);
    int ffd4 = open(FIFO4,O_RDONLY);

    struct pollfd pfds[3];

    pfds[0].fd = ffd2;
    pfds[0].events = POLLIN;

    pfds[1].fd = ffd3;
    pfds[1].events = POLLIN;

    pfds[2].fd = ffd4;
    pfds[2].events = POLLIN;

    int timeout = 5000;
    int pret, sz;
    char buf[1024];

    while(1)
    {
        pret = poll(pfds,3, timeout);
        if(pret == 0)
        {
            printf("\nTiomeout...Exit!!\n");
            break;
        }
        else
        {
            if(pfds[0].revents & POLLIN)
            {
                sz = read(ffd2,buf,1024);
                printf("Message from FIFO2: %s\n",buf);
                // printf("%s",buf);
                printf("\n");
                // fflush(stdout);
            }
            if(pfds[1].revents & POLLIN)
            {
                sz = read(ffd3,buf,1024);
                printf("Message from FIFO3: %s\n",buf);
                // printf("%s",buf);
                printf("\n");
                // fflush(stdout);
            }
            if(pfds[2].revents & POLLIN)
            {
                sz = read(ffd4,buf,1024);
                printf("Message from FIFO4: %s\n",buf);
                // printf("%s",buf);
                printf("\n");
                // fflush(stdout);
            }
        }
    }
}