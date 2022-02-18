#include<unistd.h>
#include<fcntl.h>
#include<stdio.h>


int main(int argc, char** argv)
{
    int fd = open("system_calls.txt", O_WRONLY|O_APPEND);
    for(int i=1; i<argc; i++)
    {
        write(fd,argv[i],strlen(argv[i]));
        write(fd,"\n",1);
    }

    printf("Total Count : \n");
    system("wc -w system_calls.txt");
}