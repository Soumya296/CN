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
#include<netinet/if_ether.h>
#include<net/ethernet.h>
#include<netinet/ether.h>
#include<netinet/udp.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<bits/stdc++.h>
using namespace std;

#define MAX 1024
#define PORT 8080
#define ADDRESS "ess_socket1"
#define ADDRESS1 "ess_socket2"

/*Global variables*/
int sfd;
int sig_check = 0;
int exit_check = 0;

void * read_raw(void * fd)
{
    int rawfd = *(int *) fd;
    char buf[MAX];
    struct iphdr* ip;

    while(1)
    {
        sleep(5);
        recvfrom(rawfd,buf,MAX,0,NULL,NULL);
        ip=(struct iphdr*)buf;
        if(strncmp(buf+(ip->ihl)*4, "exit", 4)==0)
        {
            exit_check = 1;
            printf("Exiting...\n");
            break;
        }
        printf("%s\n",buf+(ip->ihl*4));
    }
    pthread_exit;
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

int send_fd(int socket, int fd_to_send)
{
    struct msghdr socket_message;
    struct iovec io_vector[1];
    struct cmsghdr *control_message = NULL;
    char message_buffer[1];
    /* storage space needed for an ancillary element with a paylod of
    length is CMSG_SPACE(sizeof(length)) */
    char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];
    int available_ancillary_element_buffer_space;
    /* at least one vector of one byte must be sent */
    message_buffer[0] = 'F'; 
    io_vector[0].iov_base = message_buffer;
    io_vector[0].iov_len = 1;
    /* initialize socket message */
    memset(&socket_message, 0, sizeof(struct msghdr));
    socket_message.msg_iov = io_vector;
    socket_message.msg_iovlen = 1;
    /* provide space for the ancillary data */
    available_ancillary_element_buffer_space = CMSG_SPACE(sizeof(int));
    memset(ancillary_element_buffer, 0,
    available_ancillary_element_buffer_space);
    socket_message.msg_control = ancillary_element_buffer;
    socket_message.msg_controllen =
    available_ancillary_element_buffer_space;
    /* initialize a single ancillary data element for fd passing */
    control_message = CMSG_FIRSTHDR(&socket_message);
    control_message->cmsg_level = SOL_SOCKET;
    control_message->cmsg_type = SCM_RIGHTS;
    control_message->cmsg_len = CMSG_LEN(sizeof(int));
    *((int *) CMSG_DATA(control_message)) = fd_to_send;
    return sendmsg(socket, &socket_message, 0);
}

void handler(int sifid, siginfo_t *info, void *context)
{
	printf("Notified by next client\n");
	sig_check = 1;
}

int main()
{
    /*Handling SIGUSR1*/
    struct sigaction handle={0};
    handle.sa_flags = SA_SIGINFO;
    handle.sa_sigaction = &handler;
    sigaction(SIGUSR1,&handle,NULL);

    /*pid of previous client*/
    int fd = fileno(popen("pidof ./c1", "r"));
    char s[MAX];
    read(fd,s,MAX);
    pid_t pid_c = atoi(s);

    /*Raw socket for broadcasting*/
    int rsfd = socket(AF_INET,SOCK_RAW,10);
    int opt = 1;
    setsockopt(rsfd, IPPROTO_IP, SO_BROADCAST, &opt, sizeof(int));
    struct sockaddr_in client;
    client.sin_family=AF_INET;
    client.sin_addr.s_addr=INADDR_ANY;
    socklen_t clilen=sizeof(client);

    /*thread for continuous read of the raw socket*/
    pthread_t thread;
    pthread_create(&thread,NULL,read_raw,(void *) &rsfd);
    // pthread_join(thread,NULL);

    /*UDS To receive the berkeley socket*/
    int usfd;
    struct sockaddr_un userv_addr,ucli_addr;
    int userv_len,ucli_len;
    usfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(usfd==-1)
    perror("\nsocket ");
    bzero(&userv_addr,sizeof(userv_addr));
    userv_addr.sun_family = AF_UNIX;
    strcpy(userv_addr.sun_path, ADDRESS);
    userv_len = sizeof(userv_addr);

    /*signalling the previous client*/
    kill(pid_c,SIGUSR1);

    if(connect(usfd,(struct sockaddr *)&userv_addr,userv_len)==-1)
    perror("\n connect ");

    sfd = recv_fd(usfd);

    /*Requesting Service*/
    printf("Requesting Service\n");
    char msg[1024];
    printf("Client : ");
    scanf("%s",msg);
    send(sfd,msg,sizeof(msg),0);

    /*Announcing the service*/
    char buff[1024]="C2 is getting serviced by ESS\n";
    sendto(rsfd,buff,1024,0,(struct sockaddr*)&client,(socklen_t)clilen);


    /*Unix domain socket for socket transfer*/
    usfd = socket(AF_UNIX , SOCK_STREAM , 0);
    bzero(&userv_addr,sizeof(userv_addr));
    userv_addr.sun_family = AF_UNIX;
    strcpy(userv_addr.sun_path, ADDRESS1);
    unlink(ADDRESS1);
    userv_len = sizeof(userv_addr);
    if(bind(usfd, (struct sockaddr *)&userv_addr, userv_len)==-1)
        perror("server: bind");
    listen(usfd, 5);
    ucli_len=sizeof(ucli_addr);
    int nusfd;
    
    nusfd=accept(usfd, (struct sockaddr *)&ucli_addr, (socklen_t *) &ucli_len);

    /*wait till gets signalled by the next client*/
    while(!sig_check){}

    /*Socket Transfer*/
    send_fd(nusfd,sfd);
    printf("Socket FD sent to next client\n");

    while(!exit_check){}

    return 0;
}