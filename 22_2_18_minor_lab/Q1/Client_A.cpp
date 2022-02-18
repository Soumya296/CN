#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>

#define MAX 1024
#define PORT 8080

using namespace std;

int main()
{
    struct sockaddr_in address;
    socklen_t address_size;
    int sfd=socket(AF_INET,SOCK_STREAM,0);
    if(sfd==0)
    {
        printf("creation of socket failed\n");
    }
    address.sin_family=AF_INET;
    address.sin_port=htons(8080);
    address.sin_addr.s_addr=inet_addr("127.0.0.1");
    if(connect(sfd,(struct sockaddr *)&address,sizeof(address))<0)
    {
        printf("connection failed\n");
        return 0;
    }
    else cout<<"Connected to A for sf1\n";
    
    int sfd1=socket(AF_INET,SOCK_STREAM,0);
    if(sfd1==0)
    {
        printf("creation of socket failed\n");
    }

    address.sin_family=AF_INET;
    address.sin_port=htons(8081);
    address.sin_addr.s_addr=inet_addr("127.0.0.2");
    if(connect(sfd1,(struct sockaddr *)&address,sizeof(address))<0)
    {
        printf("connection failed\n");
        return 0;
    }
    else cout<<"Connected to A for sf2\n";

    return 0;

}