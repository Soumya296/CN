#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX 10240
#define PORT 8080

void service(int nsfd, char * buffer)
{
    bzero(buffer,MAX);
    int val_read = recv(nsfd,buffer,MAX,0);
    printf("\nReceived from client\n");
    printf("%s\n",buffer);
}

int main()
{
    int sfd, nsfd,opt=5;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[MAX];

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	}
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 

    address.sin_family = AF_INET; 
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons( PORT ); 

	if (bind(sfd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}

    if (listen(sfd, 5) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	}

    nsfd = accept(sfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if(nsfd<0)
    {
        perror("accept() failed");
        close(sfd);
        exit(0);
    }

    while(1)
    {
        sleep(5);
        service(nsfd,buffer);
    }

    return 0;
}