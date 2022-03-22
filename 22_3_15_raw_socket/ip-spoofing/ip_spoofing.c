#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netdb.h>


#define MAXLINE 4096
#define BUFFSIZE 8192
#define LISTENQ 1024
#define SERV_PORT 8080
#define PCKT_LEN 8192


int main()
{
    int rsfd = socket(AF_INET, SOCK_RAW, 10);
	if(rsfd< 0){
		perror("socket() creation error"); 
        exit(-1);
	}
	else 
        printf("\nsocket() - Created RAW socket Successfully.\n");

    char buf[PCKT_LEN];
    struct iphdr *ip = (struct iphdr *) buf;
    memset(buf, 0, PCKT_LEN);

    struct sockaddr_in  dest_in;

    dest_in.sin_family = AF_INET;
    dest_in.sin_addr.s_addr = inet_addr("127.0.0.1");

    struct hostent * host_entry = gethostbyname("csail.mit.edu");
    

    ip->ihl = 5;
	ip->version = 4;
	ip->tos = 255; 
	ip->tot_len = sizeof(struct iphdr);
	ip->id = htons(56525);
	ip->ttl = 64; 
	ip->protocol = 10; 
	ip->saddr = inet_addr(inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0])));
	ip->daddr = inet_addr("127.0.0.1");

    int opt = 1;
    if(setsockopt(rsfd, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt)) < 0){
		perror("setsockopt() error"); 
        exit(-1);
	}
	else	printf("setsockopt() is working fine...\n");

    printf("\n\nTrying to Spoof my IP Address by replacing with that of CSAIL MIT\n");
    printf("IP for csail.mit.edu is : %s\n\n", inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0])));
    printf("Source IP : %s \n",inet_ntoa(*((struct in_addr*)&ip->saddr)));
    printf("Destination IP : %s \n",inet_ntoa(*((struct in_addr*)&ip->daddr)));

    printf("\n\n");
    for(int i=0; i<5;i++)
    {
        if(sendto(rsfd, buf, ip->tot_len, 0, (struct sockaddr *)&dest_in, sizeof(dest_in)) < 0){
			perror("sendto() error"); 
            exit(-1);
		}
		else{
			printf("Packet no %d is sent successfully...\n", i+1);
			sleep(2);
		}
    }

}