#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Stream.h>
#include <bits/stdc++.h>
#include <stdio.h>
using namespace std;

int main()
{
	ACE_INET_Addr addr(8080, INADDR_LOOPBACK);
	ACE_SOCK_Stream sfd;

	ACE_SOCK_Acceptor acc(addr);

	char buffer[50]; int sz;

	while(1)
	{
		acc.accept(sfd);    
		cout<<"\nConnection Sucessful!\n"<<endl;

		sfd.recv_n(buffer,50);
		char * command;
		sprintf(command, "pidof %s", buffer);
		int fd = fileno(popen(command,"r"));
		char s[1024];
		read(fd,&s,1024);
		close(fd);
		pid_t pid_client = atoi(s);

		if(fork()==0)
		{while(1)
		{
			sz = sfd.recv_n(buffer, 50);

			if(sz==0)break;

			buffer[sz]='\0';
			printf("%d: %s\nEnter the message : \n", pid_client, buffer);

			scanf("%s", buffer);
			sfd.send_n(buffer, 50);
		}

		sfd.close();}
		else sfd.close();

	}
	return 0;
}