#include "networks.h"
using namespace std;

int main(){
   int x;
    cout<<"enter bearing number of fielder "<<endl;
    cin>>x;
    int rsfd=socket(AF_INET, SOCK_RAW, 2);	
    char buff[100];
    recvfrom(rsfd,buff,100,0,NULL,NULL);
    struct iphdr* ip;
    ip=(struct iphdr*)buff;
    printf("r generated by batsman : %s\n",buff+(ip->ihl)*4);
    int bs=atoi(buff+(ip->ihl)*4);
    if(bs%4==0 || bs%6==0)
    {

    }
    else
    {
        if((bs-x)<=5||(bs-x)>=-5)
        {
            int rsfd1=socket (AF_INET, SOCK_RAW, 3);
            int opt=1;
            setsockopt(rsfd1, IPPROTO_IP, SO_BROADCAST, &opt, sizeof(int));
            struct sockaddr_in client1;
            client1.sin_family=AF_INET;
            client1.sin_addr.s_addr=INADDR_ANY;
            char buff1[100]="catch";
            socklen_t clilen=sizeof(client1);
            sendto(rsfd1,buff1,100,0,(struct sockaddr*)&client1,(socklen_t)clilen);
            perror("sent");
        }
    }
    return 0;
}