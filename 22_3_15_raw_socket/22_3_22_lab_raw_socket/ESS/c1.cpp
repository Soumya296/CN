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
        sleep(10);
        recvfrom(rawfd,buf,MAX,0,NULL,NULL);
        ip=(struct iphdr*)buf;
        if(strncmp(buf+(ip->ihl)*4, "exit", 4)==0)
        {
            printf("Exiting...\n");
            break;
        }
        printf("%s\n",buf+(ip->ihl*4));
    }
    exit_check = 1;
    pthread_exit;
}

void handler(int sifid, siginfo_t *info, void *context)
{
	printf("Notified by next client\n");
	sig_check = 1;
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

int main()
{
    /*Handling SIGUSR1*/
    struct sigaction handle={0};
    handle.sa_flags = SA_SIGINFO;
    handle.sa_sigaction = &handler;
    sigaction(SIGUSR1,&handle,NULL);
    
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
    

    /*Berkeley Socket for service*/
    struct sockaddr_in address;
    socklen_t address_size;
    sfd=socket(AF_INET,SOCK_STREAM,0);
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

    /*Requesting Service*/
    printf("Requesting Service\n");
    char msg[1024];
    printf("Client : ");
    scanf("%s",msg);
    send(sfd,msg,sizeof(msg),0);

    /*Announcing the service*/
    char buff[1024]="C1 is getting serviced by ESS\n";
    sendto(rsfd,buff,1024,0,(struct sockaddr*)&client,(socklen_t)clilen);

    /*Unix domain socket for socket transfer*/
    int usfd;
    struct sockaddr_un userv_addr,ucli_addr;
    int userv_len,ucli_len;
    usfd = socket(AF_UNIX , SOCK_STREAM , 0);
    bzero(&userv_addr,sizeof(userv_addr));
    userv_addr.sun_family = AF_UNIX;
    strcpy(userv_addr.sun_path, ADDRESS);
    unlink(ADDRESS);
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