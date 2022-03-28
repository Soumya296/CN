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
#include<netinet/if_ether.h>
#include<net/ethernet.h>
#include<netinet/ether.h>
#include<netinet/udp.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<bits/stdc++.h>
using namespace std;

int main(){
    int rsfd=socket (AF_INET, SOCK_RAW, IPPROTO_TCP);	
    int opt=1;
    setsockopt(rsfd, IPPROTO_IP, SO_BROADCAST, &opt, sizeof(int));
    struct sockaddr_in client;
    client.sin_family=AF_INET;
    client.sin_addr.s_addr=INADDR_ANY;
    char buff[100]="testing";
    socklen_t clilen=sizeof(client);
    sendto(rsfd,buff,100,0,(struct sockaddr*)&client,(socklen_t)clilen);
    return 0;
}