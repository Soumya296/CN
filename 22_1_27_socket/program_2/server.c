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

typedef struct arg_struct{
    int sfd;
    int nsfd;
} * argptr;

void * function1 (void * args) /*echo service*/
{
    argptr arg = (argptr) args;
    int sfd = arg->sfd;
    int nsfd = arg->nsfd;
    char buf[MAX];
    recv(nsfd,buf,MAX,0);
    printf("From Client 1: %s\n",buf);
}

void * function2 (void * args) /*capitalize and send*/
{
    argptr arg = (argptr) args;
    int sfd = arg->sfd;
    int nsfd = arg->nsfd;
    char buf[MAX];
    int size = recv(nsfd,buf,MAX,0);

    for(int i=0;i<size;i++)
    {
        if(buf[i]>='A' && buf[i]<='Z')
        {
            buf[i]=buf[i]+32;
        }
    }
    send(nsfd,buf,sizeof(buf),0);
}

void * function3 (void * args) /*lowercase and send*/
{
    argptr arg = (argptr) args;
    int sfd = arg->sfd;
    int nsfd = arg->nsfd;
    char buf[MAX];
    int size = recv(nsfd,buf,MAX,0);

    for(int i=0;i<size;i++)
    {
        if(buf[i]>='A' && buf[i]<='Z')
        {
            buf[i]=buf[i]+32;
        }
    }
    send(nsfd,buf,sizeof(buf),0);
}

int main()
{
    int sfd[3],nsfd[3];

    struct sockaddr_in address[3];
    int opt = 3;
    char buffer[MAX] = {0};

    for(int i=0; i<3; i++)
    {
        if ((sfd[i] = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
        { 
            perror("socket failed"); 
            exit(EXIT_FAILURE); 
        } 
        if (setsockopt(sfd[i], SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
        { 
            perror("setsockopt"); 
            exit(EXIT_FAILURE); 
        } 

        address[i].sin_family = AF_INET; 
        address[i].sin_addr.s_addr = inet_addr("127.0.0.1"); 
        address[i].sin_port = htons( 8080+i*2 ); 

        if (bind(sfd[i], (struct sockaddr *)&address[i], sizeof(address[i]))<0) 
        { 
            perror("bind failed"); 
            exit(EXIT_FAILURE); 
        }
    }

    struct pollfd psfds[3];

    for(int i=0; i<3; i++)
    {
        psfds[i].fd = sfd[i];
        psfds[0].events = POLLIN;
    }

    int timeout = 15000;
    int pret;

    for(int i=0; i<3; i++)
    {
        if (listen(sfd[i], 3) < 0) 
        { 
            perror("listen"); 
            exit(EXIT_FAILURE); 
        } 
    }

    while(1)
    {
        pret = poll(psfds,3,timeout);
        pthread_t pid[3];
        if(pret == 0)
        {
            printf("\nTimeout...Exit!!\n");
            // break;
        }
        // else
        for(int i=0;i<3;i++){
            if(psfds[i].revents & POLLIN)
            {
                nsfd[i] = accept(psfds[i].fd,(struct sockaddr *) &address[i], sizeof(address[i]));
                if(nsfd[i] > 0)
                {
                    argptr args;
                    args = malloc(sizeof(struct arg_struct));
                    args->sfd = sfd[i];
                    args->nsfd = nsfd[i];
                    if (i == 0) pthread_create(&pid[i],NULL,&function1,args);
                    else if(i == 1) pthread_create(&pid[i],NULL,&function2,args);
                    else if(i == 2) pthread_create(&pid[i],NULL,&function3,args);
                }
            }
        }
    }

}