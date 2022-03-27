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
#define port 8080

#define ADDRESS "uds_socket"
#define No_of_Customer 2

int bsfd;
int usfd;
int csfd[No_of_Customer];
int nusfd[2]; /*one for sending to waiter and one for sending to the delivery desk*/
int price[] = {300,300,300,500,500};
int waiter_confirmation[] = {0,0};
int wc = 0;

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

void handler(int sifid, siginfo_t *info, void *context)
{
	printf("sending customer %d to the delivery desk\n",wc+1);
	send_fd(nusfd[1],csfd[wc]);
    waiter_confirmation[wc++] = 1;
}



int main()
{
	struct sigaction handle={0};
    handle.sa_flags = SA_SIGINFO;
    handle.sa_sigaction = &handler;
    sigaction(SIGUSR1,&handle,NULL);

    /*Socket for Billing Counter*/
    bsfd = socket(AF_INET, SOCK_STREAM,0);

    struct sockaddr_in addr_bill;
    addr_bill.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr_bill.sin_family = AF_INET;
    addr_bill.sin_port = htons(port);

	struct sockaddr_in  addr_c;
	int addrlen = sizeof(addr_c);

    int opt = 1;
    if(setsockopt(bsfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 

    if (bind(bsfd, (struct sockaddr *)&addr_bill, sizeof(addr_bill))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}

    if (listen(bsfd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	}

	/*Unix Domain Socket Creation for transfer of the customer socket file descriptor*/
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

	/*Connecting with Waiter and Delivery*/
	for(int i=0; i<2.; i++)
	{
		nusfd[i] = accept(usfd, (struct sockaddr *)&ucli_addr, &ucli_len);
        printf("Connection Successful on UDS\n");
	}

	/*Accepting Customer*/
	
	for(int i=0; i<No_of_Customer; i++)
	{
		if((csfd[i] = accept(bsfd,(struct sockaddr *)&addr_c, &addrlen))<0)
        {
            perror("connection failed for customer\n");
        }
        else
		{
			printf("Customer %d Joined Successfully\n",i+1);
			printf("Choose among 5 combos available\n");
			
			char buf[1024] = "combo";
			send(csfd[i],buf,sizeof(buf),0);
			recv(csfd[i],buf,1024,0);

			int combo = atoi(buf);
			printf("Combo chosen : %d\n",combo--);
			printf("Pay: %d\n", price[combo]);
			
            sprintf(buf,"pay");
            send(csfd[i],buf,sizeof(buf),0);
			if(combo<=2)
			{
				sprintf(buf,"300");
				send(csfd[i],buf,sizeof(buf),0);
			}
			else
            {   sprintf(buf,"500");
                send(csfd[i], buf,sizeof(buf),0);}

			recv(csfd[i],buf,1024,0);
            printf("Paid amounnt %d \n",atoi(buf));
			if(atoi(buf) != price[combo]){printf("exact amount not paid, terminating order!!\n");exit;}
			else{
				printf("Payment Successful\nHanding over to the waiter\n");
				send_fd(nusfd[0],csfd[i]);
			}
		}

	}

    close(bsfd);

	while(wc < No_of_Customer){}

    printf("Do Visit again...\nThank you\n\n\n");

	return 0;
}