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
#define ADDRESS "uds_socket"

using namespace std;

int GATE;
int rsfd, tsfd, fsfd, usfd;
int tcc = 0;
int fcc = 0;
int tc = 0;
int fc = 0;
int tw[5], fw[5];
int sz;

char buff[MAX];

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

int main(int argc, char * argv[])
{
    GATE = atoi(argv[1]);
    printf("Welcome to Gate number : %d\n",GATE--);

    /*pid of tm*/

    int fd = fileno(popen("pidof ./tm","r"));
    char s[MAX];
    read(fd,s,MAX);
    pid_t pid_tm = atoi(s);

    /*UDS for TM*/
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
    else printf("\nconnect to Toll gate succesful\n");

    /*BSD Sockets for vehicles*/

    tsfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in t_addr;
    
    char addr[1024];
    int opt = 3;
    sprintf(addr,"127.0.0.%d",GATE*2);

    t_addr.sin_addr.s_addr = inet_addr(addr);
    t_addr.sin_family = AF_INET;
    t_addr.sin_port = htons(8080+GATE*2);

    if(setsockopt(tsfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 

    if(bind(tsfd,(struct sockaddr *) &t_addr, sizeof(t_addr))<0)
    {
        perror("bind failed\n");
    }

    fsfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in f_addr;
    
    sprintf(addr,"127.0.0.%d",GATE*2+1);

    f_addr.sin_addr.s_addr = inet_addr(addr);
    f_addr.sin_family = AF_INET;
    f_addr.sin_port = htons(8080+GATE*2+1);

    if(setsockopt(fsfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 

    if(bind(fsfd,(struct sockaddr *) &f_addr, sizeof(f_addr))<0)
    {
        perror("bind failed\n");
    }    

    struct sockaddr_in addr_v;
    socklen_t addr_len = sizeof(addr_v);


    /*Raw socket for FT*/
    rsfd=socket (AF_INET, SOCK_RAW, 2);
    
    listen(tsfd,5);
    listen(fsfd,5);

    struct pollfd pfd[3];

    pfd[0].fd = tsfd;
    pfd[0].events = POLLIN;

    pfd[1].fd = fsfd;
    pfd[1].events = POLLIN;

    pfd[2].fd = rsfd;
    pfd[2].events = POLLIN;

    int timeout = 5000;

    while(1)
    {
        if(poll(pfd,3,timeout) == 0)
        {

        }
        else{
            for(int i=0; i<3; i++)
            {
                if(pfd[i].revents & POLLIN)
                {
                    if(pfd[i].fd == tsfd)
                    {
                        printf("New 2 wheeler came to gate\n");
                        tw[tc] = accept(tsfd,(struct sockaddr *) & addr_v, (socklen_t *) &addr_len);

                        printf("Sending to toll Manager\n");
                        send_fd(usfd,tw[tc++]);
                    }
                    else if(pfd[i].fd == fsfd)
                    {
                        printf("New 4 wheeler came to gate\n");
                        fw[fc] = accept(fsfd,(struct sockaddr *) & addr_v, (socklen_t *) &addr_len);

                        printf("Sending to toll Manager\n");
                        send_fd(usfd,fw[fc++]);
                    }
                    else if(pfd[i].fd == rsfd){
                        recvfrom(rsfd,buff,MAX,0,NULL,NULL);
                        struct iphdr* ip;
                        ip=(struct iphdr*)buff;
                    
                        int temp = atoi(buff+(ip->ihl)*4);
                        int gate = temp%10;
                        temp /= 10;
                        if(gate== GATE)
                        {
                            if(temp == 2){
                                sprintf(buff, "pass");
                                sz = send(tw[tcc++],buff,sizeof(buff),0);
                                kill(pid_tm,SIGUSR1);
                            }
                            else if(temp == 4)
                            {
                                sprintf(buff, "pass");
                                sz = send(fw[fcc++],buff,sizeof(buff),0);
                                kill(pid_tm, SIGUSR2);
                            }
                            
                        }
                    }

                }
            }
        }
    }


}