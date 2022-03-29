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
#include<poll.h>
#include<bits/stdc++.h>

#define MAX 1024


using namespace std;

char buff[MAX];
int sz;

int main(int argc, char * argv[])
{
    int no = atoi(argv[1])-1;
    int wheel = atoi(argv[2]);
    int gate = atoi(argv[3])-1;

    int sfd = socket(AF_INET, SOCK_STREAM, 0);

    char addr[1024];
    struct sockaddr_in gaddr;
    if(wheel == 2)
    {
        sprintf(addr,"127.0.0.%d",gate*2);

        gaddr.sin_addr.s_addr = inet_addr(addr);
        gaddr.sin_family = AF_INET;
        gaddr.sin_port = htons(8080+gate*2);
    }
    else if(wheel == 4)
    {
        sprintf(addr,"127.0.0.%d",gate*2+1);

    
        gaddr.sin_addr.s_addr = inet_addr(addr);
        gaddr.sin_family = AF_INET;
        gaddr.sin_port = htons(8080+gate*2+1);
    }

    if(connect(sfd,(struct sockaddr *) &gaddr,sizeof(gaddr))<0)
    {
        perror("Connection to gate failed\n");
    }
    else{
        printf("Came to Gate : %d", gate+1);
        while(1)
        {
            sz = recv(sfd,buff,MAX,0);
            if(strncmp(buff,"pass",4)==0)
            {
                printf("Got pass\n");
                break;
            }
            else if(strncmp(buff, "no",2)==0)
            {
                sprintf(buff, "%d",no);
                send(sfd,buff,sizeof(buff),0);
            }
            else if(strncmp(buff, "wheels", 6)==0)
            {
                sprintf(buff, "%d",wheel);
                send(sfd,buff,sizeof(buff),0);
            }
        }

    }
    
}