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
#define port 8080

int bsfd, sz;
char buf[1024];
int combo = -1;
char pid[1024];

void process()
{
    while(1)
    {
        sz = recv(bsfd,buf,1024,0);
        if(strncmp(buf,"combo",sizeof("combo"))==0)
        {
            if(combo == -1)
            {
                combo = rand()%4 + 1;
                printf("Ordering combo : %d\n",combo);
            }
            sprintf(buf,"%d",combo);
            send(bsfd,buf,sizeof(buf),0);
        }
        else if(strncmp(buf,"pay",sizeof("pay"))==0)
        {
            sz = recv(bsfd,buf,1024,0);
            printf("paying : %d\n",atoi(buf));
            send(bsfd,buf,sz,0);
        }
        else if(strncmp(buf,"pid",sizeof("pid"))==0)
        {
            sprintf(buf,"%d",getpid());
            printf("My coupon number : %s\n",buf);
            send(bsfd,buf,sizeof(buf),0);
        }
        else{
            printf("Received : %s\n",buf);
            if(strncmp(buf,pid,4)==0)
            printf("\n\nReceived the Correct parcel\n");
            break;
        }
    }
}

int main()
{
    sprintf(pid,"%d",getpid());

    bsfd = socket(AF_INET, SOCK_STREAM,0);

    struct sockaddr_in addr_bill;
    addr_bill.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr_bill.sin_family = AF_INET;
    addr_bill.sin_port = htons(port);

	int addrlen = sizeof(addr_bill);

    if(connect(bsfd,(struct sockaddr *)&addr_bill,addrlen)<0)
    {
        printf("Conenction failed\n");
    }
    else
    printf("Connection to biil desk Successful\n");

    process();
    return 0;
}