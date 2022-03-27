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
#include <string.h>

#define MAX 1024
#define port 8082

#define ADDRESS "uds_socket"
#define No_of_Customer 2


int customers[No_of_Customer];
int dsfd;
int ndsfd;
pid_t pid_b;

void * service(void * fd)
{
    char buf[1024] = "pid";
    char parcel[1024] = "parcel";
    int sz;
    int cfd = *(int *) fd;

    
    sprintf(buf,"pid");
    send(cfd,buf,sizeof(buf),0);

    
    
    sz = recv(cfd,buf,1024,0);
    int pid = atoi(buf);
    printf("Coupon Number : %d\n",atoi(buf));

    
    // sprintf(parcel,"%d",pid);

    sprintf(buf,"combo");
    send(cfd,buf,sizeof(buf),0);
    sz = recv(cfd,buf,1024,0);
    printf("Took Order for combo %s\n",buf);

    // strcat(parcel, strcat(" combo ",buf));
    sprintf(parcel,"%d combo : %d", pid, atoi(buf));

    printf("Preparing Food...\n");
    sleep(3);
    send(ndsfd,parcel,sizeof(parcel),0);
    printf("Parcel Sent to the delivery desk with coupon number %d\n",pid);

    /*Notify the Billing Desk*/
    kill(pid_b, SIGUSR1);
    printf("Billing Desk Notified\n");

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
     /*pid of billing desk*/
    int fd = fileno(popen("pidof ./billing_counter","r"));
    char s[1024];
    read(fd,s,1024);
    pid_b = atoi(s);

    /*Unix domain socket for customer descriptor*/
    int usfd;
    struct sockaddr_un userv_addr;
    int userv_len,ucli_len;
    usfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(usfd==-1)
    perror("\nsocket ");
    bzero(&userv_addr,sizeof(userv_addr));
    userv_addr.sun_family = AF_UNIX;
    strcpy(userv_addr.sun_path, ADDRESS);
    userv_len = sizeof(userv_addr);
    if(connect(usfd,(struct sockaddr *)&userv_addr,userv_len)==-1)
    perror("\n connect ");
    else printf("\nconnect to Billing succesful\n");


    /*Socket to send the parcel*/

    dsfd = socket(AF_INET, SOCK_STREAM,0);

    struct sockaddr_in addr_w;
    addr_w.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr_w.sin_family = AF_INET;
    addr_w.sin_port = htons(port);

    struct sockaddr_in addr_d;
	int addrlen = sizeof(addr_d);

    int opt = 1;
    if(setsockopt(dsfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 

    if (bind(dsfd, (struct sockaddr *)&addr_w, sizeof(addr_w))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}

    if (listen(dsfd, 2) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	}

    
    if((ndsfd = accept(dsfd,(struct sockaddr *)&addr_d,&addrlen))<0)
    {
        "Connection Failure with Delivery desk\n";
    }
    else
    {
        printf("Connection with delivery desk established : ");
        printf("with socket : %d\n",ndsfd);
    }

    pthread_t threads[No_of_Customer];
    
    for(int i=0; i<No_of_Customer; i++)
    {
        customers[i] = recv_fd(usfd);
        printf("\n\nCustomer socket descriptor : %d\n",customers[i]);

        pthread_create(&threads[i],NULL,service,(void *) &customers[i]);
    }

    for(int i=0;i<2; i++) pthread_join(threads[i],NULL);

}