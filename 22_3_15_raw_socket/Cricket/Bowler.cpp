#include "networks.h"
#define ADDRESS "mysocket" 
#define ADDRESS1 "mysocket1"

using namespace std;
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
	
	char element_buffer[CMSG_SPACE(sizeof(int))];
	int available_element_buffer_space;
	
	message_buffer[0]='F';
	io_vector[0].iov_base = message_buffer;
	io_vector[0].iov_len = 1;
	
	//initialise socket message
	memset(&socket_message, 0, sizeof(struct msghdr));
	socket_message.msg_iov = io_vector;
	socket_message.msg_iovlen = 1;
	
	//provide space for the ancillary data
	available_element_buffer_space = CMSG_SPACE(sizeof(int));
	memset(element_buffer, 0, available_element_buffer_space);
	socket_message.msg_control = element_buffer; 
 	socket_message.msg_controllen = available_element_buffer_space; 
	
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
    /*Unix DOmain Socket for FD sharing as ball*/

    struct sockaddr_un userv_addr,ucli_addr; 
    int userv_len; 
    int usfd = socket(AF_UNIX,SOCK_STREAM,0);
	if(usfd<=0)
		perror("socket");
	
	bzero(&userv_addr,sizeof(userv_addr));
	
	userv_addr.sun_family = AF_UNIX;
	strcpy(userv_addr.sun_path, ADDRESS);
	unlink(ADDRESS);
	userv_len = sizeof(userv_addr);
	if(bind(usfd, (struct sockaddr *)&userv_addr, userv_len)==-1)
		perror("server:bind");
	socklen_t ucli_len;
	listen(usfd,5);	
    int nusfd1= accept(usfd, (struct sockaddr*)&ucli_addr, &ucli_len);
    int fd = open("ball.txt", O_RDONLY | O_CREAT);
    cout<<"fd to represent ball "<<fd<<endl;
    /*Sending File descriptor to batsman*/
    send_fd(nusfd1,fd);
    cout<<"bowled to batsman"<<endl;

	// /*Notifying the umpire*/
	char su[1000];
	int umpire_fd=fileno(popen("pidof ./umpire","r"));
	read(umpire_fd,&su,1000);
	int pid3=atoi(su);
	kill(pid3,SIGUSR2);

    /*Connecting to the Umpire*/
    int usfd1 = socket(AF_UNIX, SOCK_STREAM, 0); 
 	if(usfd1==-1) 
	 perror("\nsocket "); 
	bzero(&userv_addr,sizeof(userv_addr)); 
	userv_addr.sun_family = AF_UNIX; 
 	strcpy(userv_addr.sun_path, ADDRESS1); 
	userv_len = sizeof(userv_addr); 
 	if(connect(usfd1,(struct sockaddr *)&userv_addr,userv_len)==-1) 
 		perror("\n connect "); 

    fd=recv_fd(usfd1);
    cout<<"ball recieved from umpire "<<fd<<endl;
}