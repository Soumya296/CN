#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include<sys/wait.h>


#define MAX 1024
#define PORT 8080

int main()
{
    int sfd, nsfd;
    int opt = 1;
    
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

    struct sockaddr_in address;
    socklen_t address_len = sizeof(address);
    char buff[MAX] = {0};

    address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT );

    if (bind(sfd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		return -1; 
	}

    if (listen(sfd, 3) < 0) 
	{ 
		perror("listen"); 
		return -1; 
	}

    nsfd = accept(sfd, (struct sockaddr *) &address, (socklen_t *) &address_len);
    if(nsfd<0)
    {
        perror("accept() failed");
        close(sfd);
        exit(0);
    }
    printf("File loc accepted\n");

    pid_t pid = fork();
    if(pid < 0) {perror("Fork unsucessfull\n"); return -1;}
    if(pid == 0)
    {
        int itfd = open("IT.txt",O_RDONLY);
        int poutfd = open("Pout.txt", O_WRONLY|O_TRUNC|O_APPEND);
        dup2(itfd,0);
        dup2(poutfd,1);

        int size = recv(nsfd, buff, MAX, 0);
        char command[1024];
        sprintf(command,"g++ -o p.exe %s",buff);

        system(command);
        execv("p.exe",NULL);
    }
    else{
        while(wait(NULL) > 0);
        printf("Parent running!!\n");

        int pout_fd = open("Pout.txt",O_RDONLY);
        int out_fd = open("OT.txt", O_RDONLY);
        char buf, msg;

        int n1 = read(pout_fd,&buf,1);
        int n2 = read(out_fd,&msg,1);

        char snd[1024];

        while(n1 != 0)
        {
            if(buf == msg)
            {
                if(buf == ' ')
                {
                    sprintf(snd,"Test case passed\n");
                    send(nsfd,snd,sizeof(snd),0);
                }
            }
            else{
                sprintf(snd,"Test case failed \n");
                send(nsfd,snd,sizeof(snd),0);
                while(1)
                {
                    n1 = read(pout_fd,&buf,1);
                    if(buf == ' ' || n1 == 0) break;
                }
                while(1)
                {
                    n2 = read(out_fd,&msg,1);
                    if(msg == ' ' || n2 == 0) break;
                }
            }
            n1 = read(pout_fd,&buf,1);
            n2 = read(out_fd,&msg,1);

            if(n1 == 0 && buf == msg)
                {
                    sprintf(snd,"Test case passed\n");
                    send(nsfd,snd,sizeof(snd),0);
                    break;
                }
            if(n2 == 0) break;
        }

        sprintf(snd,"Com");
        send(nsfd,snd,sizeof(snd),0);

    }


    // close(nsfd);
    close(sfd);
    return 0;
}