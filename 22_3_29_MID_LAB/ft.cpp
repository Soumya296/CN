#include<time.h>
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<sys/select.h>
#include<pthread.h>
#include<signal.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/shm.h>
#include<unistd.h>
#include<sys/un.h>
#include<netinet/ip.h>
#include<arpa/inet.h>
#include<errno.h>
#include<net/ethernet.h>
#include<netinet/ether.h>
#include<netinet/udp.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<bits/stdc++.h>
#define MAX 1024

using namespace std;

int database[] = {1200,2000,800,5000};
int ufd;
int rsfd, sz;
char buff[MAX];

struct sockaddr_in client;
socklen_t clilen;

struct sockaddr_in tm_addr;
socklen_t tm_len;

void service()
{
    int temp;
    int gate;
    int v;
    int w;
    int price;
    while(1)
    {
        sz = recvfrom(ufd,buff,MAX,MSG_WAITALL,(struct sockaddr *) &tm_addr, &tm_len);
        temp = atoi(buff);
        gate = temp%10;
        temp /= 10;
        v = temp%10;
        temp /= 10;
        w = temp%10;
        temp /= 10;
        price = temp;

        if(price <= database[v]) 
        {
            printf("Vechile %d is allowed to pass gate %d\n",v+1,gate);
            sprintf(buff,"%d",w*10+gate);
            sendto(rsfd,buff,sizeof(buff),0,(struct sockaddr*)&client,(socklen_t)clilen);
        }
    }
}

int main()
{
    /*UDP for vehicle info*/
    ufd = socket(AF_INET,SOCK_DGRAM,0);
    struct sockaddr_in ft_addr;

    ft_addr.sin_addr.s_addr = inet_addr("127.0.0.10");
    ft_addr.sin_family = AF_INET;
    ft_addr.sin_port = htons(8090);

    tm_len = sizeof(tm_addr);
    if(bind(ufd,(struct sockaddr * ) &ft_addr, sizeof(ft_addr))<0)
    {
        perror("Binding failed\n");
    }

    /*Raw sockets for notifying gates*/
    rsfd=socket (AF_INET, SOCK_RAW, 2);	
    int opt=1;
    setsockopt(rsfd, IPPROTO_IP, SO_BROADCAST, &opt, sizeof(int));
    
    client.sin_family=AF_INET;
    client.sin_addr.s_addr=INADDR_ANY;
    clilen=sizeof(client);

    // sleep(3);
    service();
}