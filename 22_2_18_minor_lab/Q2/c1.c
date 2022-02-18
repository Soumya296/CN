#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>

#define PORT 8080
#define MAX 1024

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

    printf("Enter message to send to server\n");
    char buff[MAX];
    scanf("%s",buff);
    send(sfd,buff,sizeof(buff),0);

    int size;

    for(;;)
    {
        size = recv(sfd, buff, MAX, 0);
        // printf("%c",buff[0]);
        if(strncmp(buff,"Com",3)== 0) break;
        printf("%s\n",buff);
        
    }

    close(sfd);
    return 0;

}