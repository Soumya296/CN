#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Connector.h>
#include <ace/UNIX_Addr.h>
#include <bits/stdc++.h>
#include <stdio.h>
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

using namespace std;

int main()
{
    ACE_INET_Addr addr_c(8082, INADDR_LOOPBACK);
    ACE_SOCK_Stream c_sfd;
    ACE_SOCK_Connector con_c;

    con_c.connect(c_sfd,addr_c);
    cout<<"Connected succesfully to the server"<<endl;

    char buf[50];
    while(1)
    {
        printf("Enter the message to send : \n");
        scanf("%s",buf);

        c_sfd.send_n(buf,sizeof(buf));
        if(strncmp(buf, "exit", 4)==0) break;
    }
    c_sfd.close_writer();
    c_sfd.close();
}