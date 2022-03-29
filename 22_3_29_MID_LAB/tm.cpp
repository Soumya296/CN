#include<time.h>
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<sys/select.h>
#include<pthread.h>
#include<signal.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/shm.h>
#include<unistd.h>
#include<sys/un.h>
#include<netinet/ip.h>
#include<arpa/inet.h>
#include<errno.h>
#include<net/ethernet.h>
#include<netinet/ether.h>
#include<netinet/udp.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<poll.h>
#include<bits/stdc++.h>
#define MAX 1024
#define No_Of_Gates 3
#define ADDRESS "uds_socket"

using namespace std;

int vfd;
int usfd;
int nusfd[No_Of_Gates];
char buff[MAX];
int sz;
int GATE;

int count_2 = 0;
int count_4 = 0;

void handler1(int sifid, siginfo_t *info, void *context)
{
    count_2++;
    printf("No of 2 wheelers passed : %d \n", count_2);
}

void handler2(int sifid, siginfo_t *info, void *context)
{
    count_4++;
    printf("No of 4 wheelers passed : %d \n", count_4);
}

int recv_fd(int socket)
{
    int sent_fd, available_ancillary_element_buffer_space;
    struct msghdr socket_message;
    struct iovec io_vector[1];
    struct cmsghdr *control_message = NULL;
    char message_buffer[1];
    char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];
    /* start clean */
    memset(&socket_message, 0, sizeof(struct msghdr));
    memset(ancillary_element_buffer, 0, CMSG_SPACE(sizeof(int)));
    /* setup a place to fill in message contents */
    io_vector[0].iov_base = message_buffer;
    io_vector[0].iov_len = 1;
    socket_message.msg_iov = io_vector;
    socket_message.msg_iovlen = 1;
    /* provide space for the ancillary data */
    socket_message.msg_control = ancillary_element_buffer;
    socket_message.msg_controllen = CMSG_SPACE(sizeof(int));
    if(recvmsg(socket, &socket_message, MSG_CMSG_CLOEXEC) < 0)
    return -1;
    if(message_buffer[0] != 'F')
    {
    /* this did not originate from the above function */
    return -1;
    }
    if((socket_message.msg_flags & MSG_CTRUNC) == MSG_CTRUNC)
    {
    /* we did not provide enough space for the ancillary element array */
    return -1;
    } 
    /* iterate ancillary elements */
    for(control_message = CMSG_FIRSTHDR(&socket_message);
    control_message != NULL;
    control_message = CMSG_NXTHDR(&socket_message, control_message))
    {
    if( (control_message->cmsg_level == SOL_SOCKET) &&
    (control_message->cmsg_type == SCM_RIGHTS) )
    {
    sent_fd = *((int *) CMSG_DATA(control_message));
    return sent_fd;
    }
    }
    return -1;
}

int main()
{
    /*Handling signals*/

    struct sigaction handle1={0};
    handle1.sa_flags = SA_SIGINFO;
    handle1.sa_sigaction = &handler1;
    sigaction(SIGUSR1,&handle1,NULL);

    struct sigaction handle2={0};
    handle2.sa_flags = SA_SIGINFO;
    handle2.sa_sigaction = &handler2;
    sigaction(SIGUSR2,&handle2,NULL);
    
        /*UDS for gates*/
    struct sockaddr_un userv_addr,ucli_addr;
    int userv_len;
    usfd = socket(AF_UNIX , SOCK_STREAM , 0);
    bzero(&userv_addr,sizeof(userv_addr));
    userv_addr.sun_family = AF_UNIX;
    strcpy(userv_addr.sun_path, ADDRESS);
    unlink(ADDRESS);
    userv_len = sizeof(userv_addr);
    if(bind(usfd, (struct sockaddr *)&userv_addr, userv_len)==-1)
        perror("server: bind");
    listen(usfd, 3);
    socklen_t ucli_len=sizeof(ucli_addr);


    struct pollfd pfd[No_Of_Gates];
	/*Connecting with gates*/
	for(int i=0; i<No_Of_Gates; i++)
	{
		nusfd[i] = accept(usfd, (struct sockaddr *)&ucli_addr, &ucli_len);
        pfd[i].fd = nusfd[i];
        pfd[i].events = POLLIN;
	}

    /*UDP for FT*/
    int ufd = socket(AF_INET,SOCK_DGRAM,0);
    struct sockaddr_in  ft_addr;

    ft_addr.sin_addr.s_addr = inet_addr("127.0.0.10");
    ft_addr.sin_family = AF_INET;
    ft_addr.sin_port = htons(8090);


    int timeout = 5000;
    int w, price, v;
    int temp = 0;

    while(1)
    {
        if(poll(pfd,No_Of_Gates,timeout)==0)
        {
        }
        else{
            for(int i=0; i<No_Of_Gates; i++)
            {
                if(pfd[i].revents & POLLIN)
                {
                    GATE = i;
                    vfd = recv_fd(nusfd[i]);
                    if(vfd>0) printf("Received the vehicle from gate : %d\n",GATE);

                    sprintf(buff, "no");
                    send(vfd,buff,sizeof(buff),0);
                    sz = recv(vfd,buff,MAX,0);
                    v = atoi(buff);

                    sprintf(buff, "wheels");
                    send(vfd,buff,sizeof(buff),0);
                    sz = recv(vfd,buff,MAX,0);
                    w = atoi(buff);

                    if(w == 2) price = 200;
                    else price = 500;

                    temp = price;
                    temp = temp*10 +w;
                    temp = temp*10 +v;
                    temp = temp*10+GATE;

                    sprintf(buff, "%d", temp);
                       
                    sendto(ufd, buff, sizeof(buff), MSG_CONFIRM, (const struct sockaddr *) &ft_addr,sizeof(ft_addr));
                    printf("Info sent to Fast Tag\n");
                }
            }
        }
    }

}