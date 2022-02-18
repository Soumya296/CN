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

int send_int(int num, int fd)
{
    int32_t conv = htonl(num);
    char *data = (char*)&conv;
    int left = sizeof(conv);
    int rc;
    do {
        rc = write(fd, data, left);
        if (rc < 0) {
            return -1;
        }
        else {
            data += rc;
            left -= rc;
        }
    }
    while (left > 0);
    return 0;
}

int main()
{
    struct sockaddr_in address;
    socklen_t address_size;
    int sfd=socket(AF_INET,SOCK_STREAM,0);
    if(sfd==0)
    {
        printf("creation of socket failed\n");
    }
    address.sin_family=AF_INET;
    address.sin_port=htons(8080);
    address.sin_addr.s_addr=inet_addr("127.0.0.1");
    if(connect(sfd,(struct sockaddr *)&address,sizeof(address))<0)
    {
        printf("connection failed\n");
        return 0;
    }

    printf("Requesting Service from server 2\n");
    send_int(3,sfd);

    char buff[MAX];
    int size;

    for(;;)
    {
        size = recv(sfd, buff, MAX, 0);
        // if(strncmp(buff,"Com",3)== 0) break;
        printf("%s",buff);
    }

    close(sfd);
    return 0;
}