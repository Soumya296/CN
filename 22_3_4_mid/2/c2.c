#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>

#define MAX 1024
#define port_c 8090

int main()
{
    int c_sfd = socket(AF_INET,SOCK_STREAM, 0);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port_c);
    address.sin_addr.s_addr = inet_addr("127.0.0.2");

    if(connect(c_sfd,(struct sockaddr *)&address,sizeof(address))<0)
    {
        printf("connection failed\n");
        return 0;
    }
    else printf("\nConnection to Server Successful\n");

    char buf[MAX] = "Client 2\n";
    send(c_sfd,buf,sizeof(buf),0);
    char msg[MAX];

    while(1)
    {
        printf("Enter the msg\n");
        scanf("%s",buf);
        send(c_sfd,buf,sizeof(buf),0);
        recv(c_sfd,msg,MAX,0);
        if(strncmp(msg,"AS",2)==0 || strncmp(msg,"ser",3)==0)
        {
            sleep(5);
        }
    }
}