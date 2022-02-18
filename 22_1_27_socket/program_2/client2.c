#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include<sys/types.h>
#include<poll.h>
#include<pthread.h>

#define MAX 1024

int main()
{
    struct sockaddr_in add;
    int sfd=socket(AF_INET,SOCK_STREAM,0);
    if(sfd==0)
    {
        perror("socket failed"); 
        exit(EXIT_FAILURE);
    }
    add.sin_family=AF_INET;
    add.sin_addr.s_addr=inet_addr("127.0.0.1");
    add.sin_port=htons(8082);
    if(connect(sfd,(struct sockaddr *)&add,sizeof(add))<0)
    {
        printf("connection failed\n");
    }
    char msg[MAX];
    char buf[MAX];
    printf("Client : ");
    scanf("%s",msg);
    send(sfd,msg,strlen(msg),0);
    if(strcmp(msg,"exit")==0)
    recv(sfd,buf,MAX,0);
    printf("Server : %s\n",buf);
    return 0;
}