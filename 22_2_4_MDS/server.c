#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <poll.h>

#define MAX 1024
#define N 5
#define D 4


int receive_int(int *num, int fd)
{
    int32_t ret;
    char *data = (char*)&ret;
    int left = sizeof(ret);
    int rc;
    do {
        rc = read(fd, data, left);
        if (rc <= 0) {
            return -1;
        }
        else {
            data += rc;
            left -= rc;
        }
    }
    while (left > 0);
    *num = ntohl(ret);
    return 0;
}

int main()
{
    int sfd, nsfd[N];

    struct sockaddr_in address;
    int opt = N;
    char buffer[MAX] = {0};

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket creation failed\n"); 
        exit(EXIT_FAILURE); 
    } 
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    { 
        perror("setsockopt\n"); 
        exit(EXIT_FAILURE); 
    } 

    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    address.sin_port = htons(8080); 

    if (bind(sfd, (struct sockaddr *)&address, sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }

    if (listen(sfd, N) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    }

    for(int i = 0; i<N; i++)
    {
        nsfd[i] = accept(sfd, (struct sockaddr *) &address, sizeof(address));
    }

    int dfd[D] = {0};
    int num_service[N];

    for(int i=0; i<N; i++)
    {
        receive_int(&num_service[i],nsfd[i]);
        if(dfd[num_service[i]] == 0) 
        {
            char command[MAX];
            sprintf(command,"./D%d",num_service[i]);
            dfd[num_service[i]] = fileno(popen(command,"r"));
        }
    }

    for (int i=0; i<D; i++)
    {
        if(dfd[i] > 0)
        {
            int sz = read(dfd[i],buffer,MAX);
            printf("%s",buffer);
            if(sz > 0)
            {
                for(int j=0; j<N; j++)
                {
                    if(num_service[j] == i)
                    {
                        send(nsfd[j],buffer,sz,0);
                    }
                }
            }
        }
    }


}