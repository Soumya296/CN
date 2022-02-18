#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <bits/stdc++.h>
#include <signal.h>
#include <poll.h>

#define MAX 80
#define PORT 8080

using namespace std;

int main()
{
    int fd = fileno(popen("pidof ./S", "r"));
    char s[1024];
    read(fd,&s,1000);
    pid_t pid_S = atoi(s);


    int sfd1,sfd2, nsfd[5], valread;
    int sf = 0;

	struct sockaddr_in address; 
	int opt = 3; 
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 

	if ((sfd1 = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
    if ((sfd2 = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 

	if (setsockopt(sfd1, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	}
    if (setsockopt(sfd2, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	}

    address.sin_family = AF_INET; 
	address.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	address.sin_port = htons( PORT );

    if (bind(sfd1, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}

    address.sin_family = AF_INET; 
	address.sin_addr.s_addr = inet_addr("127.0.0.2"); 
	address.sin_port = htons( PORT +1);

    if (bind(sfd2, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}

    if (listen(sfd1, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	}
    if (listen(sfd2, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	}

    struct pollfd pfd[2];
    memset(pfd, 0 , sizeof(pfd));

    pfd[0].fd = sfd1;
    pfd[0].events = POLLIN;

    pfd[1].fd = sfd2;
    pfd[1].events = POLLIN;

    int timeout = (3 * 60 * 1000);

    while(1)
    {
        int rc = poll(pfd,2,timeout);
        if(rc<=0) break;

        for(int i=0; i<2; i++)
        {
            if(pfd[i].revents && POLLIN)
            {
                if(pfd[i].fd == sfd1)
                {
                    nsfd[sf++] = accept(sfd1,(struct sockaddr *)&address, (socklen_t*)&addrlen);
                    if(nsfd<0)
                    {
                        perror("accept() failed");
                        close(sfd1);
                        exit(0);
                    }
                    kill(pid_S,SIGUSR1);
                }
                else if(pfd[i].fd == sfd2)
                {
                    nsfd[sf++] = accept(sfd2,(struct sockaddr *)&address, (socklen_t*)&addrlen);
                    if(nsfd<0)
                    {
                        perror("accept() failed");
                        close(sfd2);
                        exit(0);
                    }
                    kill(pid_S,SIGUSR2);
                }
            }
        }

    }

}