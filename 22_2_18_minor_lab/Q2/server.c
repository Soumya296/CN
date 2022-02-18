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
#define NUM_Service 3

void service1(int nsfd)
{
    char buf[MAX];
    int sz = recv(nsfd,buf,MAX,0);
    for (int i=0; i<sz; i++)
    {
        if(buf[i] >= 'a' && buf[i] <= 'z') buf[i] -= 32;
    }
    send(nsfd,buf,MAX,0);
    send(nsfd,"Com",sizeof("Com"),0);
}

void service2(int nsfd)
{
    char buf[MAX];
    int sz = recv(nsfd,buf,MAX,0);
    for (int i=0; i<sz; i++)
    {
        if(buf[i] >= 'A' && buf[i] <= 'Z') buf[i] += 32;
    }
    send(nsfd,buf,MAX,0);
    send(nsfd,"Com",sizeof("Com"),0);
}

void service3(int nsfd)
{
    char buf[MAX];
    int sz = recv(nsfd,buf,MAX,0);
    printf("Echoing message: %s\n",buf);
}

void Service(int nsfd, int service_no)
{
    switch (service_no){
        case 0: service1(nsfd); break;
        case 1: service2(nsfd); break;
        case 2: service3(nsfd); break;
    }
}

int main()
{
    // create as many sockets as the total number of services
    // here 3 services and 3 sfd

    int sfd[NUM_Service];
    int nsfd[5];
    int service[5];
    int count = 0;
    int opt = 3;

    struct sockaddr_in address[NUM_Service];
    socklen_t address_len = sizeof(address[0]);

    for (int i=0; i<NUM_Service; i++)
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
	    address[i].sin_port = htons( PORT + i);

        if (bind(sfd[i], (struct sockaddr *)&address[i], sizeof(address[i]))<0) 
        { 
            perror("bind failed"); 
            return -1; 
        }

        if (listen(sfd[i], 3) < 0) 
        { 
            perror("listen"); 
            return -1; 
        }
    }

    // int address_len = sizeof(address[0]);

    for (int i=0; i<NUM_Service; i++)
    {
        nsfd[count] = accept(sfd[i], (struct sockaddr *) &address[i], (socklen_t *) &address_len);
        if(nsfd<0)
        {
            perror("accept() failed");
            close(sfd);
            exit(0);
        }
        service[count++] = i;
    }


    struct pollfd pfd[count];

    for (int i=0; i<count; i++)
    {
        pfd[i].fd = nsfd[i];
        pfd[i].events = POLLIN;
    }
    int timeout = (3 * 60 * 1000);

    while(1)
    {
        int rc = poll(pfd,count,timeout);
        if(rc<=0) break;

        for(int i=0; i<count; i++)
        {
            if(pfd[i].revents && POLLIN)
            {
                Service(nsfd[i],service[i]);
            }
        }
    }

}