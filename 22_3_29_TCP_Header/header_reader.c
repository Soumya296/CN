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

    struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(bind(rsfd,(struct sockaddr*)&addr,sizeof(addr))<0)
	{
		perror("Could not bind");exit(0);
	}

    int sz;
    char buff[1024];

    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(clilen);

    while(1)
    {
        if((sz = recvfrom(rsfd,buff,MAX,0,(struct sockaddr *) &cli_addr, &clilen))<0)
        {
            perror("Error in Message receipt\n");
            break;
        }

        struct iphdr* ip;
        ip = (struct iphdr*)buff;
        struct tcphdr* tcp;
        tcp = (struct tcphdr*)(buff+(ip->ihl)*4);
        printf("***********TCP Header***********\n");
        printf("Destination port: %d\n",ntohs(tcp->dest));
        printf("Source Port: %d\n",ntohs(tcp->source));
        printf("Sequence Number: %d\n",ntohs(tcp->seq));
        printf("Acknowledgement No: %d\n",ntohs(tcp->ack_seq));
        printf("FIN: %d\n",(int)tcp->fin);
        printf("ACK: %d\n",(int)tcp->ack);
        printf("URG: %d\n",(int)tcp->urg);
        printf("SYN: %d\n",(int)tcp->syn);
        printf("RESET: %d\n",(int)tcp->rst);
        printf("PUSH: %d\n",(int)tcp->psh);

        printf("Window Size: %d\n",(int)tcp->window);

        printf("The data: %s\n",buff+(ip->ihl)*4+(tcp->doff)*4);

        printf("***********TCP Header***********\n\n");


    }

}