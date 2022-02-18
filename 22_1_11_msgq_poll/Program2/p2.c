#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<poll.h>

char * FIFO2 = "FIFO2";

int main()
{
    mkfifo(FIFO2, 0666);
    int ffd = open(FIFO2,O_WRONLY);

    char buf[1024];
    int sz;

    while(1)
    {
    printf("Enter the Input\n");
    sz = read(0,buf,1024);
    if(sz ==1 && buf[0] == '\n')
    {
        printf("Null input...exit!\n");
        break;
    }
        
    write(ffd,buf,sz);
    }
}