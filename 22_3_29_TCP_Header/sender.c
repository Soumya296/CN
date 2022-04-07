#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/select.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#define MAX 1024

int main()
{
    int rsfd = socket(AF_INET,SOCK_RAW,IPPROTO_TCP);
    if(rsfd<0)
    {
        perror("Socket Creation Failed\n");
    }

    int opt=1;
    setsockopt(rsfd, IPPROTO_IP, SO_BROADCAST, &opt, sizeof(int));
    
    char buf[MAX]= "hello\n";

    struct sockaddr_in client;
    client.sin_addr.s_addr = inet_addr("127.0.0.1");
    client.sin_family = AF_INET;
    socklen_t clilen = sizeof(client);
    sendto(rsfd,buf,MAX,0,(struct sockaddr *) &client, clilen);
}