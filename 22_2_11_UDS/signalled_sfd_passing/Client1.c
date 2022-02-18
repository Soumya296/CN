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

#define MAX 10240
#define PORT 8080

struct msg_buf {
    long mtype;
    char mtext[108];
};

char * message_queue = "./queue";
char path[108];
int c_pid, msqid;
struct msg_buf message;

void handler(int sifid, siginfo_t *info, void *context)
{
    c_pid = info->si_pid;

    printf("Received signal from process with PID : %d\n",c_pid);
    /* Read the UD Socket address */

    msgrcv(msqid,&message,sizeof(message.mtext),(long)c_pid,0);
    sprintf(path,"%s",message.mtext);

    printf("PID : %ld\n",message.mtype);
    printf("Address : %s\n",message.mtext);
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
    /* Customizing signal behaviour */

    struct sigaction handle={0};
    handle.sa_flags = SA_SIGINFO;
    handle.sa_sigaction = &handler;
    sigaction(SIGUSR1,&handle,NULL);

    /*Message Queue Part Starts*/

    key_t key = ftok(message_queue,0);
    msqid = msgget(key,IPC_CREAT|0666);

    message.mtype = (long)getpid();
    msgsnd(msqid,&message,sizeof(message.mtext),0);

    /* Berkeley Socket Part Starts */

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

    /*Requesting Service*/

    sleep(3);
    printf("Requesting Service\n");
    char msg[MAX];
    printf("Client : ");
    scanf("%s",msg);
    send(sfd,msg,MAX,0);

    /* Sharing FD */

    int usfd;
    struct sockaddr_un userv_addr;
    int userv_len,ucli_len;
    usfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(usfd==-1)
    perror("\nsocket ");
    bzero(&userv_addr,sizeof(userv_addr));
    userv_addr.sun_family = AF_UNIX;
    strcpy(userv_addr.sun_path, path);
    userv_len = sizeof(userv_addr);
    if(connect(usfd,(struct sockaddr *)&userv_addr,userv_len)==-1)
    perror("\n connect ");
    else printf("\nconnect succesful");

    send_fd(usfd,sfd);
    printf("Socket FD sent to next client\n");
}