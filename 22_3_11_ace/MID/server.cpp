#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Stream.h>
#include <ace/UNIX_Addr.h>
#include <bits/stdc++.h>
#include <stdio.h>
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
#include <pthread.h>

#define ADDRESS "ud_socket1"
using namespace std;

int send_fd(const ACE_HANDLE socket, const ACE_SOCK_Stream fd_to_send)
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
    *((ACE_SOCK_Stream *) CMSG_DATA(control_message)) = fd_to_send;
    return sendmsg(socket, &socket_message, 0);
}

ACE_SOCK_Stream recv_fd(const ACE_HANDLE socket)
{
    ACE_SOCK_Stream sent_fd, available_ancillary_element_buffer_space;
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
    sent_fd = *((ACE_SOCK_Stream*) CMSG_DATA(control_message));
    return sent_fd;
    }
    }
    cout<<"failure on fd transfer\n";
    return -1;
}

int main()
{
    ACE_UNIX_Addr addr(ADDRESS);
    ACE_SOCK_Stream usfd;
    ACE_SOCK_Acceptor acc(addr);

    acc.accept(usfd);
    cout<<"Connection sucessful on UNIX Socket"<<endl;


    ACE_INET_Addr addr_as(8080, INADDR_LOOPBACK);
    ACE_SOCK_Stream as_sfd;
    ACE_SOCK_Acceptor acc_as(addr_as);

    acc_as.accept(as_sfd);
    cout<<"\nConnection Sucessful with alternate server\n"<<endl;

    ACE_INET_Addr addr_c(8082, INADDR_LOOPBACK);
    ACE_SOCK_Stream c_sfd;
    ACE_SOCK_Acceptor acc_c(addr_c);

    acc_c.accept(c_sfd);
    cout<<"\nConnection Sucessful with client\n"<<endl;

    char buf[50];
    int sz = c_sfd.recv_n(buf,50);
    buf[sz]='\0';

    if(sz > 0)
    {
        cout<<"Received from client : ";
        cout<<buf<<endl;
    }

    cout<<"Going for maintanence"<<endl;

    cout<<"Sending the socket descriptor to the alternate server\n";
    cout<<"Status of sending : "<<send_fd(usfd.get_handle(),c_sfd)<<endl;

    sleep(10);


    int fd = fileno(popen("pidof ./as","r"));
    char s[1024];
    sz = read(fd,s,1024);
    int pid_as = atoi(s);

    kill(pid_as,SIGUSR1);
    
    // c_sfd = recv_fd(usfd.get_handle());
    cout<<endl;
    cout<<"ready to serve...\n";
    cout<<"Receieved from client\n";
    while(1)
    {
        sz = c_sfd.recv_n(buf,50);
        if(sz<0) break;
        buf[sz]='\0';
        if(strncmp(buf, "exit", 4)==0) break;
        cout<<buf<<endl;
    }

    as_sfd.close_writer();
    as_sfd.close();
    usfd.close();
    c_sfd.close();

}