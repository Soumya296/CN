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

#define MAX 1024
#define port 8080

int main()
{
    int sfd = socket(AF_INET,SOCK_STREAM,0);
    if(sfd == 0) perror("Socket failure\n");

    int opt = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 

    struct sockaddr_in address;
    int addrlen = sizeof(address);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(port);

    if (bind(sfd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}

    if (listen(sfd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	}

    int nsfd_perf = accept(sfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    char buf[1024];

    printf("\nSHowing Performance : \n");
    while(1)
    {
        recv(nsfd_perf,buf,1024,0);
        if(strncmp(buf,"com",3)==0)
        {
            printf("Performance Completed\n");
            break;
        }
        printf("%s",buf);
    }

    /*Getting the pid of anchor to get the judges' Process ID*/
    int fd = fileno(popen("pidof ./anchor", "r"));
    char s[1024];
    read(fd,&s,1024);
    close(fd);
    pid_t pid_anchor = atoi(s);

    kill(pid_anchor, SIGUSR1);

    sleep(3);
    printf("\nSHowing Reply : \n");

    while(1)
    {
        recv(nsfd_perf,buf,1024,0);
        if(strncmp(buf,"com",3)==0) 
        {
            printf("Replies Completed\n");
            break;
        }
        printf("%s",buf);
    }    

    close(nsfd_perf);
    close(sfd);

    return 0;
}