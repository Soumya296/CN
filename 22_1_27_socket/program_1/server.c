
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#define MAX 80
#define PORT 8080

void GetPeerName()
{
    int s, sa_len;
    struct sockaddr_in sa;
    sa_len = sizeof(sa);

    if (getpeername(s, &sa, &sa_len) == -1) {
      perror("getsockname() failed");
      return -1;
   }

   printf("Foreign IP address is: %s\n", inet_ntoa(sa.sin_addr));
   printf("Foreign port is: %d\n", (int) ntohs(sa.sin_port));
}

int main(int argc, char const * argv[])
{
    int num;
    if (argc >= 2)
      num = atoi(argv[1]);
   else
      num = 1;

    int sfd, nsfd, valread; 
	struct sockaddr_in address; 
	int opt = 3; 
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 

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
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 

	if (bind(sfd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}

    int rc = getsockname(sfd,(struct sockaddr *)&address, sizeof(address));
    char ip[100];
    inet_ntop(AF_INET,&address.sin_addr,ip,sizeof(ip));
    printf("local ip : %s\n",ip);
    printf("local port %d \n",ntohs(address.sin_port));

    if (listen(sfd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 

    for(int i=0; i<num; i++)
    {
        nsfd = accept(sfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if(nsfd<0)
        {
            perror("accept() failed");
            close(sfd);
            exit(0);
        }
        printf("accept completed successfully\n");
        getpeername(sfd,(struct sockaddr*)&address,sizeof(address));
        inet_ntop(AF_INET,&address.sin_addr,ip,sizeof(ip));
        printf("foreign ip : %s\n",ip);
        printf("foreign port %d \n",ntohs(address.sin_port));

        valread = recv( nsfd , buffer, 1024,0);
        printf(buffer);
        printf("\n");
        close(nsfd);
    }
    return 0;
    
}