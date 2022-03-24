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

#define MAX 1024
#define port_s 8080
#define port_c 8090
#define ADDRESS "ud_socket1"

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



/*Service Glabal Var*/
int SERVICE = 1;

void * service(void * fd)
{
    int nsfd = * (int *) fd;
    char buf[MAX];
    int sz;
    while(1)
    {
        if(SERVICE == 0) break;
        recv(nsfd,buf,MAX,0);
        printf("From %s\n",buf);
    }
    pthread_exit(NULL);
}

int main()
{
    /*Socket creation for Alternate Server*/
    int s_sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(s_sfd == 0) perror("Socket()\n");

    struct sockaddr_in address;
    int addrlen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_port = htons(port_s);
    address.sin_addr.s_addr = inet_addr("127.0.0.1");

    int opt = 1;
    if (setsockopt(s_sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 

    if (bind(s_sfd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}

    if (listen(s_sfd, 1) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	}

    /*Socket for clients*/
    int c_sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(c_sfd == 0) perror("Socket()\n");


    address.sin_family = AF_INET;
    address.sin_port = htons(port_c);
    address.sin_addr.s_addr = inet_addr("127.0.0.2");

    opt = 1;
    if (setsockopt(c_sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 

    if (bind(c_sfd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}

    if (listen(c_sfd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	}

    int as_sfd = accept(s_sfd,(struct sockaddr *)&address, (socklen_t*)&addrlen);

    /*Unix Domain socket Creation*/
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
    listen(usfd, 3);
    ucli_len=sizeof(ucli_addr);
    int nusfd;
    nusfd=accept(usfd, (struct sockaddr *)&ucli_addr, &ucli_len);

    
    int nsfd[2];
    nsfd[0] = accept(c_sfd,(struct sockaddr *)&address, (socklen_t*)&addrlen);
    nsfd[1] = accept(c_sfd,(struct sockaddr *)&address, (socklen_t*)&addrlen);

    pthread_t threads[2];
    for(int i = 0; i<2; i++)
    {
        pthread_create(&threads[i],NULL,service,&nsfd[i]);
    }

    for(int i = 0; i<2; i++)
    {
        pthread_join(threads[i],NULL);
    }

    sleep(3);
    /*Maintenance Time*/
    SERVICE = 0;
    char buf[MAX] = "AS";
    send(nsfd[0],buf,sizeof(buf),0);
    send(nsfd[1],buf,sizeof(buf),0);
    send(as_sfd,buf,sizeof(buf),0);

    for(int i=0; i<2; i++)
    {
        send_fd(nusfd,nsfd[i]);
    }
    printf("Going for Maintenance\n");
    
    SERVICE = 1;
    printf("Ready to Serve\n");
    sprintf(buf,"red");
    send(as_sfd,buf,sizeof(buf),0);

    sleep(3);

    for(int i = 0; i<2; i++)
    {
        pthread_create(&threads[i],NULL,service,&nsfd[i]);
    }

    for(int i = 0; i<2; i++)
    {
        pthread_join(threads[i],NULL);
    }

    return 0;
}