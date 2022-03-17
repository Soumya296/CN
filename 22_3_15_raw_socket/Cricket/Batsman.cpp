#include "networks.h"
#include <time.h>
#define ADDRESS "mysocket" 
#define ADDRESS2 "mysocket2"

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

int run(char * buf, int sz)
{
	int speed = atoi(strtok(buf,"\t"));
	int spin = atoi(strtok(NULL,"\t"));
    srand(speed+spin);
    int r=rand()%41;
    cout<<"random number generated : "<<r<<endl;
    return r;
}

int main()
{
    struct sockaddr_un userv_addr,ucli_addr; 
    int userv_len,ucli_len; 
    int usfd = socket(AF_UNIX, SOCK_STREAM, 0); 
 	if(usfd==-1) 
	 perror("\nsocket "); 
	bzero(&userv_addr,sizeof(userv_addr)); 
	userv_addr.sun_family = AF_UNIX; 
 	strcpy(userv_addr.sun_path, ADDRESS); 
	userv_len = sizeof(userv_addr); 
 	if(connect(usfd,(struct sockaddr *)&userv_addr,userv_len)==-1) 
 		perror("\n connect "); 

    /*int fd=recv_fd(usfd);
    printf("ball recieved from baller \n");*/
    int usfd1 = socket(AF_UNIX,SOCK_STREAM,0);
	if(usfd1<=0)
		perror("socket");
	
	bzero(&userv_addr,sizeof(userv_addr));
	
	userv_addr.sun_family = AF_UNIX;
	strcpy(userv_addr.sun_path, ADDRESS2);
	unlink(ADDRESS2);
	userv_len = sizeof(userv_addr);
	if(bind(usfd1, (struct sockaddr *)&userv_addr, userv_len)==-1)
		perror("server:bind");
	socklen_t ucli1_len;
	listen(usfd1,5);	
    int fd=recv_fd(usfd);
    printf("ball bowled from Bowler : %d\n",fd);

    char su[1000];
    int umpire_fd=fileno(popen("pidof ./umpire","r"));
    read(umpire_fd,&su,1000);
    int pid3=atoi(su);
    kill(pid3,SIGUSR1);


    char c[100];
    char buf[1024];
    int sz = read(fd,buf,1024);
    cout<<"speed and spin of bowler\n";
    cout<<buf<<endl;
    int r=run(buf,sz);
    int rsfd=socket (AF_INET, SOCK_RAW, 2);	
    int opt=1;
    setsockopt(rsfd, IPPROTO_IP, SO_BROADCAST, &opt, sizeof(int));
    struct sockaddr_in client;
    client.sin_family=AF_INET;
    client.sin_addr.s_addr=INADDR_ANY;
    char buff[100];
    sprintf(buff,"%d",r);
    socklen_t clilen=sizeof(client);
    sendto(rsfd,buff,100,0,(struct sockaddr*)&client,(socklen_t)clilen);


    int nusfd1= accept(usfd1,NULL,NULL);
	send_fd(nusfd1,fd);
    printf("ball sent to umpire sent\n");
    return 0;
}