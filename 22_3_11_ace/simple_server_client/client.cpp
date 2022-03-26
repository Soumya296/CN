#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <bits/stdc++.h>
#include <stdio.h>
using namespace std;

int main()
{
	ACE_INET_Addr addr(8080, INADDR_LOOPBACK);
	ACE_SOCK_Stream sfd;
	ACE_SOCK_Connector con;

	con.connect(sfd, addr);
	cout<<"Connected Successfully to the Server!"<<endl;

	char buffer[50]  = "./client"; 
	int sz;
	printf("Sending process name for the pid retrieval...\n");
	sfd.send_n(buffer, 50);

	while(1)
	{
		printf("Enter the message : \n");
		scanf("%s", buffer);
		sfd.send_n(buffer, 50);

		printf("Received from Server : ");
		sz = sfd.recv_n(buffer, 50);
		buffer[sz]='\0';
		printf("%s\n", buffer);
	}
	return 0;
}